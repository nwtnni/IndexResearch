/*
 * Copyright (c) 2024, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <sys/stat.h>
#include <libpmem.h>
#include <mutex>
#include "util.h"

const char* pm_pool = nullptr;

void* memory_alloc(size_t len, bool pmem, int ch = 78) {
  void* ret;
  if(pmem) {
    size_t map_len;
    int is_pmem;
    ret = pmem_map_file(pm_pool, len, PMEM_FILE_CREATE | PMEM_FILE_TMPFILE, DEFFILEMODE, &map_len, &is_pmem);
    if(map_len != len) {
      std::cout << "unknown error: pmem map failed" << std::endl;
      exit(-1);
    }
  } else {
    if(numa_available() >= 0)
      ret = numa_alloc_local(len);
    else
      ret = malloc(len);
  }

  if(ret == nullptr) {
    std::cout << "unknown error: memory alloc failed" << std::endl;
    exit(-1);
  }

  memset(ret, ch, len);
  return ret;
}

void memory_copy(void* dst, void* src, size_t len, int type) {
  switch(type) {
    case 0: // memcpy    read
      memcpy(dst, src, len);
      break;
    case 1: // pmemcpy    read
      pmem_memcpy(dst, src, len, PMEM_F_MEM_NOFLUSH);
      break;
    case 2: // memcpy with flush and fence,  write
      memcpy(dst, src, len);
      pmem_persist(dst, len);
      break;
    case 3: // temporal store with flush and fence, write
      pmem_memcpy(dst, src, len, PMEM_F_MEM_TEMPORAL);
      break;
    case 4: // ntstore with fence,        write
      pmem_memcpy(dst, src, len, PMEM_F_MEM_NONTEMPORAL);
      break;
    default:
      std::cout << "invalid memory_copy type" << std::endl;
      exit(-1);
  }
}

static thread_local void* thd_buffer = nullptr;

const size_t kilobyte = 1024ul;
const size_t gigabyte = 1024ul * 1024 * 1024;
size_t mlen = 64 * gigabyte;
size_t max_block = 16 * kilobyte;

int main(int argc, char* argv[]) {
  if(argc < 5) {
    std::cout << "-- nthd, run time(s), pmem(0/1), numa node(0/1)" << std::endl;
    exit(-1);
  }

  int run_time, nthd, pmem, node;
  nthd = std::stoi(argv[1]);
  run_time = std::stoi(argv[2]);
  pmem = std::stoi(argv[3]);
  node = std::stoi(argv[4]);
  std::cout << "-- nthd: " << nthd << ", run time: " << run_time << ", mem: " << (pmem ? "PMEM" : "DRAM")
            << ", numa node: " << node << ", mem size: " << mlen / gigabyte << std::endl;

  if(node == 0) pm_pool = "/mnt/pmem0/";
  else if(node == 1) pm_pool = "/mnt/pmem1/";
  else {
    std::cout << "error node" << std::endl;
    exit(-1);
  }

  util::PinningMap pin;
  pin.pinning_thread(node, 0, pthread_self());

  std::vector<std::string> sequential{"random ", "sequential "};
  std::vector<std::string> ops{"read, libc memcpy", "read, pmemcpy without flush/fence",
                               "write, libc memcpy + pmem_persist", "write, temporal pmemcpy",
                               "write, non-temporal pmemcpy"};

  void* mem = memory_alloc(mlen, pmem);
  for(size_t block_len = 64; block_len <= max_block; block_len <<= 1) {
    std::cout << "-- block size: " << block_len << std::endl;
    for(int seq : {0, 1}) { // random or sequential memory access
      for(int type : {0, 1, 2, 3, 4}) { // operation type
        std::cout << "  -- " << sequential[seq] << ops[type] << std::flush;
        std::vector<std::thread> workers;
        std::vector<double> throughput;
        std::mutex lock;
        double total_tpt = 0;
        pin.reset_pinning_counter();
        for(int tid = 0; tid < nthd; tid++) {
          workers.push_back(std::thread([&](int tid) {
            pin.pinning_thread_continuous(pthread_self());
            if(thd_buffer == nullptr) {
              thd_buffer = memory_alloc(max_block, false, 56);
            }
            // avoid thread access memory in the same address
            size_t cpy_cnt = 0, thd_mlen = mlen / nthd / block_len * block_len;
            void* thd_mem = (char*) mem + thd_mlen * tid;
            tid = pthread_self();

            util::UnifGenerator offset(size_t(0), std::numeric_limits<size_t>::max(),
                                       util::hash(tid));
            util::Timer<> timer;
            timer.start();
            while(true) {
              size_t off = seq ? cpy_cnt : offset();// util::hash(cpy_cnt + tid);
              off = (off * block_len) % thd_mlen;
              void* block = (char*) thd_mem + off;
              switch(type) {
                case 0:
                case 1:
                  memory_copy(thd_buffer, block, block_len, type);
                  break;
                case 2:
                case 3:
                case 4:
                  memory_copy(block, thd_buffer, block_len, type);
                  break;
                default:
                  std::cout << "invalid type" << std::endl;
                  exit(-1);
              }
              if(cpy_cnt++ % 1000 == 0 && timer.duration_s() > run_time) break;
            }
            long drt = timer.duration_s();

            if(numa_available() >= 0)
              numa_free(thd_buffer, max_block);
            else
              free(thd_buffer);

            std::lock_guard guard(lock);
            throughput.push_back(double(cpy_cnt * block_len) / gigabyte / drt);
          }, tid));
        }

        for(int tid = 0; tid < nthd; tid++) {
          workers[tid].join();
          total_tpt += throughput[tid];
        }
        std::cout << ", bandwidth: " << total_tpt << std::endl;
      }
    }
  }

  return 0;
}