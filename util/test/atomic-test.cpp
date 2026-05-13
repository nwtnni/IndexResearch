#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include "util.h"

using namespace util;

enum class ReqType { READ, UPDATE };

struct LoadType {
  double read_ratio;
  double update_ratio;
};

static LoadType read_only{.read_ratio = 1, .update_ratio = 0};

static LoadType read_heavy{.read_ratio=0.8, .update_ratio=0.2};

static LoadType balanced{.read_ratio=0.5, .update_ratio = 0.5};

static LoadType update_heavy{.read_ratio = 0.2, .update_ratio = 0.8};

static LoadType update_only{.read_ratio= 0, .update_ratio = 1};

LoadType workload_type(int type) {
  switch(type) {
    case 0:
      return read_only;
    case 1:
      return read_heavy;
    case 2:
      return balanced;
    case 3:
      return update_heavy;
    case 4:
      return update_only;
    default:
      std::cerr << "-- no such workload type" << std::endl;
      exit(-1);
  }
}

struct Request {
  ReqType req_type;
  uint32_t rec_id; // record index
};

class WorkloadsGenerator {
  ZipfGenerator<uint32_t> generator_;

 public:
  WorkloadsGenerator(uint32_t rec_num, double skew) : generator_(0, rec_num, skew) {}

  void generate(std::vector<Request>& requests, uint32_t load_num, LoadType ratio) {
    int read_thd = ratio.read_ratio * 100;
    UnifGenerator<int> ratio_gen(1, 100);
    requests.reserve(load_num);
    for(uint32_t i = 0; i < load_num; i++) {
      Request req;
      req.rec_id = generator_();
      if(ratio_gen() <= read_thd) req.req_type = ReqType::READ;
      else req.req_type = ReqType::UPDATE;
      requests.push_back(req);
    }
  }
};

class alignas(64) RecordType {
  std::atomic<uint64_t> record_;
  std::mutex lock_;

  static constexpr std::memory_order load_order = std::memory_order_acquire;
  static constexpr std::memory_order store_order = std::memory_order_release;

 public:
  uint64_t read() { return record_.load(load_order); }

  uint64_t write() {
    uint64_t expect = record_.load(load_order);
    uint64_t desire = expect + 1;
    while(!record_.compare_exchange_strong(expect, desire)) {
      desire = expect + 1;
    }
    return expect;

//    return record_.fetch_add(1);

//    std::lock_guard guard(lock_);
//    (*(uint64_t*) &record_)++;
//    return *(uint64_t*) &record_;
  }
};

class alignas(64) DataBase {
  RecordType* records_;

 public:
  DataBase(uint32_t size) {
    records_ = (RecordType*) malloc(size * sizeof(RecordType));
    memset(records_, 0, size * sizeof(RecordType));
  }

  uint64_t operates(Request req) {
    if(req.req_type == ReqType::READ) {
      return records_[req.rec_id].read();
    }
    return records_[req.rec_id].write();
  }

  ~DataBase() { free(records_); }
};


void run_driver(DataBase& table, std::vector<Request>& loads, int nthd, int run_time) {
  PinningMap pin;
  std::vector<std::thread> workers;
  std::vector<double> throughput;
  std::mutex out_lock;
  double total_throughput = 0;
  for(int tid = 0; tid < nthd; tid++) {
    workers.push_back(std::thread([&](int tid) {
      pin.pinning_thread_continuous(pthread_self());
      size_t begin = loads.size() * tid / nthd;
      size_t range = loads.size() * (tid + 1) / nthd - begin;
      Timer timer;
      timer.start();
      size_t req_id = 0, result = 0;
      while(true) {
        result += table.operates(loads[req_id++ % range + begin]);
        if(req_id % 100000 == 0 && timer.duration_s() >= run_time) break;
      }
      long drt = timer.duration_us();
      std::lock_guard guard(out_lock);
      std::cout << "-- thd: " << tid << ", tpt: " << double(req_id) / drt <<
                ", no sense: " << result << std::endl;
      throughput.push_back(double(req_id) / drt);
    }, tid));
  }

  for(int tid = 0; tid < nthd; tid++) {
    workers[tid].join();
    total_throughput += throughput[tid];
  }

  std::cout << "-- total throughput: " << total_throughput << std::endl;
}


int main(int argc, char* argv[]) {
  if(argc < 7) {
    std::cerr << "-- record num, load num, load type, load skew, thread num, run time (second)" << std::endl;
    std::cerr << "-- load type: 0-read_only, 1-read_heavy, 2-balanced, 3-update_heavy, 4-update-only" << std::endl;
    exit(-1);
  }
  uint32_t record_num = std::stoi(argv[1]);
  uint32_t load_num = std::stoi(argv[2]);
  int load_type = std::stoi(argv[3]);
  double load_skew = std::stod(argv[4]);
  int nthd = std::stoi(argv[5]);
  int run_time = std::stoi(argv[6]);

  LoadType type = workload_type(load_type);
  printf("-- record num: %i, load num: %i, load skew: %f, thread num: %i,"
         " run time: %i\n", record_num, load_num, load_skew, nthd, run_time);
  printf("-- load type: read ratio: %f, update ratio: %f\n", type.read_ratio, type.update_ratio);
  fflush(stdout);

  PinningMap pin;
  pin.pinning_thread(0, 0, pthread_self());
  DataBase table(record_num);
  WorkloadsGenerator gen(record_num, load_skew);
  std::vector<Request> loads;
  gen.generate(loads, load_num, type);

  run_driver(table, loads, nthd, run_time);
  return 0;
}