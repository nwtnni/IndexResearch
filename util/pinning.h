/*
 * Copyright (c) 2022-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_PINNING_H
#define UTIL_PINNING_H

#include <iostream>
#include <atomic>
#include <vector>
#include <numa.h>
#include <sys/sysinfo.h>
#include "macro.h"

namespace util {

class PinningMap {
 private:
  int nthreads_; // available/online physical threads (processors or logical cores) number
  std::atomic<int> pinning_counter_;
  std::vector<std::vector<int>> thread_map_;
  // logical thread id to physical thread id map, from lower numa node to higher numa node

 private:
  void arg_check(int socket_id, int tid) const {
    // socket_id: numa node id, tid: logical thread id in a numa node
    bool invalid = false;
    if(socket_id >= thread_map_.size()) {
      std::cerr << GRAPH_FONT_RED << "[ERROR]: socket id must be less than socket num"
                << GRAPH_ATTR_NONE << std::endl;
      invalid = true;
    }
    if(tid >= thread_map_[socket_id].size()) {
      std::cerr << GRAPH_FONT_RED << "[ERROR]: thread id must be less than thread per socket"
                << GRAPH_ATTR_NONE << std::endl;
      invalid = true;
    }
    if(invalid) exit(-1);
  }

 public:
  PinningMap() : pinning_counter_(0) {
    nthreads_ = get_nprocs(); // available processors
    int ncores = get_nprocs_conf(); // total physical threads
    if(nthreads_ != ncores) {
      std::cerr << GRAPH_FONT_RED << "[ERROR]: the case is not handled, where some processors are offline"
                << GRAPH_ATTR_NONE << std::endl;
      exit(-1);
    }

    if(numa_available() < 0) { // not support numa
      thread_map_.emplace_back();
      for(int tid = 0; tid < nthreads_; tid++)
        thread_map_[0].emplace_back(tid);
    } else {
      int numa = numa_num_configured_nodes();
      for(int nid = 0; nid < numa; nid++)
        thread_map_.emplace_back();
      for(int tid = 0; tid < nthreads_; tid++) {
        int nid = numa_node_of_cpu(tid);
        if(nid == -1) {
          std::cerr << GRAPH_FONT_RED << "[ERROR]: invalid cpu" << GRAPH_ATTR_NONE << std::endl;
          exit(-1);
        }
        thread_map_[nid].emplace_back(tid);
      }
    }
  }

  int pinning_thread(int socket_id, int tid, pthread_t pthread) const {
    // socket_id: numa node id, tid: logical thread id in a numa node
    arg_check(socket_id, tid);
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_map_[socket_id][tid], &cpuset);
    return pthread_setaffinity_np(pthread, sizeof(cpu_set_t), &cpuset);
  }

  // reset pinning_counter_ as n ( socket_id * thread_per_socket_ + tid ), so function
  // pinning_thread_continuous(pthread_t pthread) starts pinning from logical thread id n
  void reset_pinning_counter(int socket_id = 0, int tid = 0) {
    arg_check(socket_id, tid);
    int thread_count = 0;
    for(int id = 0; id < socket_id; id++)
      thread_count += thread_map_[id].size();
    pinning_counter_ = thread_count + tid;
  }

  int pinning_thread_continuous(pthread_t pthread) {
    // pinning logical threads to hardware threads within one socket if the socket have any other
    // hardware threads haven't been pinned to, otherwise pinning threads to the next socket
    // (first pin threads to hardware cores, then to logical cores sharing the same hardware cores)
    int tid = (pinning_counter_++) % nthreads_;
    int socket_id = 0;
    while(thread_map_[socket_id].size() <= tid) {
      tid -= thread_map_[socket_id].size();
      socket_id++;
    }

    arg_check(socket_id, tid);
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_map_[socket_id][tid], &cpuset);
    int ret = pthread_setaffinity_np(pthread, sizeof(cpu_set_t), &cpuset);
    return ret;
  }

  // available/online physical threads (processors or logical cores) number
  int processor_number() const { return nthreads_; }

  // numa nodes (socket) number
  int numa_number() const { return thread_map_.size(); }

  // set numa memory allocation policy, page interleaving by default
  // page interleaving: allocate in a round-robin fashion from all available numa nodes
  // else local allocation: allocate on the node on which the task is currently executing
  // other specific policies (allocate on specific nodes, memory policies specified for
  // a range of shared memory attached using shmat or mmap) , refer to `man numa` for more information
  static void set_numa_policy(bool interleaved = true) {
    if(numa_available() >= 0) {
      if(interleaved) { // interleaved policy on all numa nodes
        numa_set_interleave_mask(numa_all_nodes_ptr);
      } else { // local allocation policy
        numa_set_localalloc();
      }
    }
  }
};

}

#endif //UTIL_PINNING_H
