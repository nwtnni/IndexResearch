/*
 * Copyright (c) 2024-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_GENERATOR_H
#define UTIL_GENERATOR_H

#include <random>
#include <cmath>
#include <cassert>
#include "hash.h"

namespace util {

using util::hash;

// random engine using hash function, uint64
class HashEngine {
  uint64_t state_;

 public:
  HashEngine() : state_(0) {}

  HashEngine(uint64_t seed) : state_(seed) {}

  uint64_t max() { return std::numeric_limits<uint64_t>::max(); }

  uint64_t operator()() { return hash(state_++); }
};

// linear congruential random engine, uint32
class LCEngine {
  std::minstd_rand engine_;

 public:
  LCEngine() : engine_() {}

  LCEngine(uint64_t seed) : engine_(seed) {}

  uint64_t max() { return std::numeric_limits<uint32_t>::max(); }

  uint64_t operator()() { return engine_(); }
};

//mersenne twister random engine, uint64
class MTEngine {
  std::mt19937_64 engine_;

 public:
  MTEngine() : engine_() {}

  MTEngine(uint64_t seed) : engine_(seed) {}

  uint64_t max() { return std::numeric_limits<uint64_t>::max(); }

  uint64_t operator()() { return engine_(); }
};

// random engine using hardware entropy source, uint32
class HardwareEngine {
  std::random_device engine_;

 public:
  HardwareEngine() : engine_() {}

  HardwareEngine(uint64_t seed) : engine_() {}

  uint64_t max() { return std::numeric_limits<uint32_t>::max(); }

  uint64_t operator()() { return engine_(); }
};

// default random number engine
template<typename Engine = MTEngine>
class RandomEngine {
  Engine engine_;

 public:
  RandomEngine() : engine_() {}

  RandomEngine(uint64_t seed) : engine_(seed) {}

  uint64_t max() { return engine_.max(); }

  uint64_t operator()() { return engine_(); }
};


// uniform distribution generator [lower, lower + range)
template<typename IntType = uint64_t,
  typename RandEngine = RandomEngine<MTEngine>>
class UnifGenerator {
  IntType lower_;
  uint64_t range_;
  RandEngine engine_;

 public:
  explicit UnifGenerator(IntType lower, IntType range)
    : lower_(lower), range_(range), engine_() {
    assert(range > 0 && range < engine_.max());
  }

  explicit UnifGenerator(IntType lower, IntType range, uint64_t seed)
    : lower_(lower), range_(range), engine_(seed) {
    assert(range > 0 && range < engine_.max());
  }

  IntType operator()() { return engine_() % range_ + lower_; }
};


// Zipf distribution generator [lower, lower + range)
// The first n items (hotspot) follow a Zipfian distribution, the remaining follows a uniform distribution
// reference "Quickly Generating Billion-Record Synthetic Databases, Jim Gray et al., SIGMOD 1994"
template<typename IntType = uint64_t,
  typename RandEngine = RandomEngine<MTEngine>>
class ZipfGenerator {
  IntType lower_;
  uint64_t range_;
  RandEngine engine_;
  // the cumulative distribution function of zeta distribution -> uint64
  std::vector<uint64_t> bound_;

 private:
  // approximate value of Riemann zeta function
  double zeta_estimate(uint64_t n, double skew) {
    double zeta = 0;
    for(uint64_t i = 1; i <= n; i++)
      zeta += 1.0 / std::pow(i, skew);
    return zeta;
  }

  void init(double skew, double hotspot) {
    assert(range_ > 0 && range_ < engine_.max());
    // The first n terms of the zipf distribution (hotspot)
    uint64_t least_size = 1024 * 128;
    uint64_t nterm = range_ * hotspot;
    if(nterm < least_size) nterm = least_size;
    if(range_ < least_size) nterm = range_;

    double zeta = zeta_estimate(range_, skew), cdf = 0;
    bound_.reserve(nterm);
    for(uint64_t i = 1; i <= nterm; i++) {
      cdf += 1.0 / (zeta * std::pow(i, skew));
      bound_.push_back(cdf * engine_.max());
    }
  }

 public:
  ZipfGenerator(IntType lower, IntType range, double skew = 0.99, double hotspot = 0.001)
    : lower_(lower), range_(range), engine_() { init(skew, hotspot); }

  ZipfGenerator(IntType lower, IntType range, uint64_t seed, double skew = 0.99, double hotspot = 0.001)
    : lower_(lower), range_(range), engine_(seed) { init(skew, hotspot); }

  IntType operator()() {
    uint64_t unif = engine_();
    size_t nterm = bound_.size();
    size_t lid = 0, hid = nterm;
    while(lid < hid) {
      size_t mid = (lid + hid) / 2;
      if(unif == bound_[mid]) return lower_ + mid;
      if(unif < bound_[mid]) hid = mid;
      else lid = mid + 1;
    }
    assert(lid == hid);
    if(hid < nterm) return lower_ + hid;

    // items other than the first n terms
    return unif % (range_ - nterm) + nterm + lower_;
  }
};

}


#endif //UTIL_GENERATOR_H
