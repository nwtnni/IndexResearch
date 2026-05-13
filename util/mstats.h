/*
 * Copyright (c) 2022-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_MSTATS_H
#define UTIL_MSTATS_H

#include <iostream>
#include <iomanip>
#include <jemalloc/jemalloc.h>
#include "macro.h"

namespace util {

/** jemalloc stats, not completely up-to-date statistics due to thread caching */
class MemStats {
  // --enable-stats was specified during build configuration
  bool stats_{};
  // for refreshing the data from which the mallctl functions report values
  uint64_t epoch_{};
  // total number of bytes allocated by the application.
  size_t allocated_{};
  // total number of bytes in active pages allocated, greater than or equal to allocated, not include metadata
  size_t active_{};
  // total number of bytes dedicated to metadata, including bootstrap-sensitive allocator metadata, allocation metadata
  size_t metadata_{};
  // maximum number of bytes in physically resident data pages mapped by allocator, including allocator
  // metadata, pages backing active allocations, unused dirty pages; maximum rather than precise, because
  // pages may not actually be physically resident, for example, a page allocated but has not been touched
  size_t resident_{};
  // total number of bytes in active extents mapped by the allocator, greater than active, not include inactive
  // extents, even those that contain unused dirty pages, which means no strict ordering between this and resident
  size_t mapped_{};
  // total number of bytes in virtual memory mappings that were retained rather than being returned to the
  // operating system, retained virtual memory is typically untouched, decommitted, or purged, it has no strongly
  // associated physical memory; retained memory is excluded from mapped memory statistics, e.g. stats.mapped
  size_t retained_{};

 public:
  // dump jemalloc statistics to stderr
  static void dump_stats() { malloc_stats_print(nullptr, nullptr, nullptr); }

  // flush thread-specific cache to arena, release all cached objects and
  // internal data structure associated with the calling thread's tcache
  // thread-specific cache will be flushed when a thread exit normally
  static void thread_flush_cache() {
    if(mallctl("thread.tcache.flush", nullptr, nullptr, nullptr, 0) != 0) {
      std::cerr << GRAPH_FONT_RED << "[ERROR]: mallctl(thread.tcache.flush) unknown error"
                << GRAPH_ATTR_NONE << std::endl;
      exit(-1);
    }
  }

  MemStats() {
    size_t sz = sizeof(stats_);
    if(mallctl("config.stats", &stats_, &sz, nullptr, 0) != 0) {
      std::cerr << GRAPH_FONT_RED << "[ERROR]: mallctl(config.stats) unknown error"
                << GRAPH_ATTR_NONE << std::endl;
      exit(-1);
    }
    if(!stats_) {
      std::cerr << GRAPH_FONT_YELLOW << "[INFO]: 'enable-stats' was not specified during build configuration"
                << GRAPH_ATTR_NONE << std::endl;
    }
    thread_flush_cache();
    update_stats();
  }

  ~MemStats() = default;

  void update_stats(bool dump = false, bool head = true) {
    size_t es = sizeof(uint64_t), sz = sizeof(size_t);
    int error = 0;
    if(stats_) {
      error |= mallctl("epoch", &epoch_, &es, &epoch_, es);
      error |= mallctl("stats.allocated", &allocated_, &sz, nullptr, 0);
      error |= mallctl("stats.active", &active_, &sz, nullptr, 0);
      error |= mallctl("stats.metadata", &metadata_, &sz, nullptr, 0);
      error |= mallctl("stats.resident", &resident_, &sz, nullptr, 0);
      error |= mallctl("stats.mapped", &mapped_, &sz, nullptr, 0);
      error |= mallctl("stats.retained", &retained_, &sz, nullptr, 0);
      if(error != 0) {
        std::cerr << GRAPH_FONT_RED << "[ERROR]: mallctl(stats.*) unknown error"
                  << GRAPH_ATTR_NONE << std::endl;
        exit(-1);
      }
    }
    if(dump) {
      int width = 12;
      char fill = ' ';
      if(head) {
        std::cout << "-- " << std::left << std::setw(width) << std::setfill(fill) << "allocated";
        std::cout << std::left << std::setw(width) << std::setfill(fill) << "active";
        std::cout << std::left << std::setw(width) << std::setfill(fill) << "metadata";
        std::cout << std::left << std::setw(width) << std::setfill(fill) << "resident";
        std::cout << std::left << std::setw(width) << std::setfill(fill) << "mapped";
        std::cout << std::left << std::setw(width) << std::setfill(fill) << "retained" << std::endl;
      }
      std::cout << "   " << std::left << std::setw(width) << std::setfill(fill) << allocated_;
      std::cout << std::left << std::setw(width) << std::setfill(fill) << active_;
      std::cout << std::left << std::setw(width) << std::setfill(fill) << metadata_;
      std::cout << std::left << std::setw(width) << std::setfill(fill) << resident_;
      std::cout << std::left << std::setw(width) << std::setfill(fill) << mapped_;
      std::cout << std::left << std::setw(width) << std::setfill(fill) << retained_ << std::endl;
    }
  }

  size_t allocated() const { return allocated_; }

  size_t active() const { return active_; }

  size_t metadata() const { return metadata_; }

  size_t resident() const { return resident_; }

  size_t mapped() const { return mapped_; }

  size_t retained() const { return retained_; }

  // thread specific stats (precise statistics/up-to-date)
  size_t thread_allocated() const {
    // Notes: alloc_ptr points to a jemalloc private statistic variable
    uint64_t* alloc_ptr = nullptr;
    if(stats_) {
      size_t sz = sizeof(uint64_t*);
      if(mallctl("thread.allocatedp", &alloc_ptr, &sz, nullptr, 0) != 0) {
        std::cerr << GRAPH_FONT_RED << "[ERROR]: mallctl(thread.allocatedp) unknown error"
                  << GRAPH_ATTR_NONE << std::endl;
        exit(-1);
      }
    }
    if(alloc_ptr) return *alloc_ptr;
    return 0; // --enable-stats was not specified
  }

  size_t thread_deallocated() const {
    uint64_t* dealloc_ptr = nullptr;
    if(stats_) {
      size_t sz = sizeof(uint64_t*);
      if(mallctl("thread.deallocatedp", &dealloc_ptr, &sz, nullptr, 0) != 0) {
        std::cerr << GRAPH_FONT_RED << "[ERROR]: mallctl(thread.deallocatedp) unknown error"
                  << GRAPH_ATTR_NONE << std::endl;
        exit(-1);
      }
    }
    if(dealloc_ptr) return *dealloc_ptr;
    return 0; // --enable-stats was not specified
  }
};

}

#endif //UTIL_MSTATS_H
