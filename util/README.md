# Utility Library
This repository includes some common used compiler intrinsic function, 
memory reclamation algorithms, inline assembly, memory allocator statistics, etc.

**Under continuous update ......**

## headers
* common.h includes:
  * memory/compiler fence
  * some useful inline assembly code
  * intrinsics: prefetch, bit manipulation, cache flush instructions, non-temporal stores
* epoch.h: an epoch-based memory reclamation, supporting type erase, for example: `retire([p](){delete p;});`
* generator.h: uniform and zipfian distribution data generator
* hash.h: some useful hash function, including cpp standard hash, jenkins hash, murmur hash,
  faster hash, city hash
* hwinfo.h: some class about reading hardware information on Intel platform, such as, 
  cache info - cache size, latency, etc.
* log.h: macro based log print for debugging
* macro.h: some useful customized macro, gcc/g++ intrinsics
* mstats.h: some encapsulation about statistics interface of jemalloc
* pinning.h: pin threads to hardware cores
* simd.h: some simple encapsulation about simd instructions
* strutil.h: some useful interfaces about string
* timer.h: encapsulation of standard system clock
* type.h: some customized type

