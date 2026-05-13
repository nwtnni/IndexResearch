#include <iostream>
#include <vector>
#include "mstats.h"

using namespace util;

int main() {
  MemStats stats;
  std::cerr << "-- thread.allocated  deallocated: " <<
            stats.thread_allocated() << " " << stats.thread_deallocated() << std::endl;
  stats.update_stats(true);
  int times = 1001;
  std::vector<void*> mem;
  for(int i = 0; i < times; i++) {
    mem.push_back(malloc(i * 16));
    if(i % 100 == 0) stats.update_stats(true, false);
  }
  for(int i = 0; i < times; i++) {
    free(mem[i]);
    if(i % 100 == 0) stats.update_stats(true, false);
  }
  std::cerr << "-- thread.allocated  deallocated: " <<
            stats.thread_allocated() << " " << stats.thread_deallocated() << std::endl;
  stats.thread_flush_cache();
  stats.update_stats(true);
  std::cerr << "-- thread.allocated  deallocated: " << stats.thread_allocated()
            << " " << stats.thread_deallocated() << "\n\n" << std::endl;

  stats.dump_stats();

  return 0;
}