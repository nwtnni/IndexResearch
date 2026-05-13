#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include "lock.h"
#include "timer.h"

using namespace util;

struct alignas(64) Counter {
  size_t num;

  Counter() : num(0) {}
};

int main(int argc, char* argv[]) {
  int nthd = std::stoi(argv[1]);
  size_t ntimes = std::stoul(argv[2]);

  MutexLock<SpinLock> lock;
  std::vector<std::thread> workers;
  std::vector<Counter> counters(nthd);
  Timer timer;
  timer.start();
  size_t cnt = 0;
  for(int tid = 0; tid < nthd; tid++) {
    workers.push_back(std::thread([&](int tid) {
      for(size_t i = 0; i < ntimes; i++) {
        LockGuard guard(lock);
        counters[tid].num++;
      }
    }, tid));
  }
  for(int tid = 0; tid < nthd; tid++) {
    workers[tid].join();
    cnt += counters[tid].num;
  }
  long drt = timer.duration_us();
  double tpt = double(nthd * ntimes) / drt;
  std::cout << "[Throughput]: " << tpt << ", cnt:" << cnt << std::endl;

  return 0;
}