/*
 * Copyright (c) 2022-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_HASH_H
#define UTIL_HASH_H

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <tuple>
#include "internal/hash-internal.h"

namespace util {

/** An impressive and very very detailed hash function benchmark https://github.com/rurban/smhasher.git
 * almost all the publicly available hash function implementations can be found there, salute */

namespace util_hash {

/**================ C++ standard hash ================*/
inline size_t standard_hash(void* key, size_t len) {
  const size_t seed = 0xc70f6907UL; // seed used in std::hash<string>
  return std::_Hash_bytes(key, len, seed);
}

/* directly return key as hash value */
inline size_t standard_hash(uint64_t key) {
  return std::hash<uint64_t>()(key);
}

inline size_t standard_hash(uint32_t key) {
  return std::hash<uint32_t>()(key);
}

/**================== jenkins hash ==================*/
inline size_t jenkins_hash(void* key, size_t len) {
  return internal::jenkins_hash(key, len);
}

inline size_t jenkins_hash(uint64_t key) {
  return jenkins_hash(&key, 8);
}

inline size_t jenkins_hash(uint32_t key) {
  return jenkins_hash(&key, 4);
}

inline std::pair<uint64_t, uint64_t> spooky_hash128(void* key, size_t len) {
  return internal::spooky_hash(key, len);
}

inline size_t spooky_hash(void* key, size_t len) {
  return spooky_hash128(key, len).first;
}

inline size_t spooky_hash(uint64_t key) {
  return spooky_hash(&key, 8);
}

inline size_t spooky_hash(uint32_t key) {
  return spooky_hash(&key, 4);
}

/**================== murmur hash ==================*/

inline size_t murmur_hash(void* key, size_t len) {
  return internal::murmur_hash64(key, len);
}

inline size_t murmur_hash(uint64_t key) {
  return murmur_hash(&key, 8);
}

inline size_t murmur_hash(uint32_t key) {
  return murmur_hash(&key, 4);
}

/**================== faster hash ==================*/
inline size_t faster_hash(void* key, size_t len) {
  return internal::faster_hash(key, len);
}

inline size_t faster_hash(uint64_t key) {
  return internal::faster_hash(key);
}

inline size_t faster_hash(uint32_t key) {
  return faster_hash(uint64_t(key));
}

/**=================== city hash ===================*/
inline std::pair<uint64_t, uint64_t> city_hash128(void* key, size_t len) {
  return internal::city_hash128((const char*) key, len);
}

inline size_t city_hash(void* key, size_t len) {
  return internal::city_hash64((const char*) key, len);
}

inline size_t city_hash(uint64_t key) {
  return city_hash(&key, 8);
}

inline size_t city_hash(uint32_t key) {
  return city_hash(&key, 4);
}

}

inline size_t hash(const void* key, size_t len) {
  return util_hash::city_hash((void*) key, len);
}

inline size_t hash(uint64_t key) {
  return util_hash::faster_hash(key);
}

inline size_t hash(uint32_t key) {
  return util_hash::faster_hash(key);
}

inline size_t hash(int64_t key) {
  return hash(uint64_t(key));
}

inline size_t hash(int32_t key) {
  return hash(uint32_t(key));
}

}

#endif //UTIL_HASH_H
