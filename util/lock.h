/*
 * Copyright (c) 2025-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_LOCK_H
#define UTIL_LOCK_H

#include <atomic>
#include <cassert>
#include <mutex>
#include "common.h"

namespace util {

class SpinLock {
  std::atomic<uint64_t> control_;

  static constexpr uint64_t kLockBit = 0x01;
  static constexpr uint64_t kInitCode = 0x00;

  static constexpr std::memory_order load_order = std::memory_order_acquire;
  static constexpr std::memory_order store_order = std::memory_order_release;

 public:
  SpinLock() : control_(kInitCode) {}

  ~SpinLock() = default;

  SpinLock(const SpinLock&) = delete;

  SpinLock& operator=(const SpinLock&) = delete;

  void lock() {
    while(true) {
      uint64_t expected = control_.load(load_order);
      uint64_t desired = expected | kLockBit;
      if((expected & kLockBit) == 0 &&
         control_.compare_exchange_strong(expected, desired))
        break;
      cpu_pause();
    }
  }

  bool try_lock() {
    uint64_t expected = control_.load(load_order);
    if((expected & kLockBit) == kLockBit) return false;
    uint64_t desired = expected | kLockBit;
    return control_.compare_exchange_strong(expected, desired);
  }

  void unlock() {
    assert((control_.load(load_order) & kLockBit) == kLockBit);
    control_.fetch_sub(kLockBit);
  }
};


/**
 * @brief thinly wraps std::mutex or SpinLock
 * */
template<typename LockType = std::mutex>
class MutexLock {
  LockType lock_;

 public:
  MutexLock() = default;

  ~MutexLock() = default;

  MutexLock(const MutexLock&) = delete;

  MutexLock& operator=(const MutexLock&) = delete;

  void lock() { lock_.lock(); }

  bool try_lock() { return lock_.try_lock(); }

  void unlock() { lock_.unlock(); }
};


template<typename LockType>
class LockGuard {
  LockType& lock_;

 public:
  explicit LockGuard(LockType& lock) : lock_(lock) { lock_.lock(); }

  ~LockGuard() { lock_.unlock(); }

  LockGuard(const LockGuard&) = delete;

  LockGuard& operator=(const LockGuard&) = delete;
};


/**
 * @brief version validation combined with lock
 * */
class VerLock {
  std::atomic<uint64_t> code_;
  // Layout: | 47-bit Version | 1-bit lock | 16-bit reserved |

  static constexpr uint64_t kInitCode = 0x00ul;
  static constexpr uint64_t kLockBit = 0x01'0000ul;
  static constexpr uint64_t kVerAtom = kLockBit * 2;
  static constexpr uint64_t kVerMask = ~(kVerAtom - 1) & -1ul;
  static constexpr uint64_t kRestMask = kLockBit - 1;

  static constexpr std::memory_order load_order = std::memory_order_acquire;
  static constexpr std::memory_order store_order = std::memory_order_release;

 public:
  VerLock() : code_(kInitCode) {};

  ~VerLock() = default;

  VerLock(const VerLock&) = delete;

  VerLock& operator=(const VerLock&) = delete;

  /**
   * @brief acquire exclusive lock, subsequent writers and readers are blocked
   * */
  void lock_exclusive() {
    while(true) {
      uint64_t expected = code_.load(load_order);
      uint64_t desired = expected | kLockBit;
      if((expected & kLockBit) == 0 &&
         code_.compare_exchange_strong(expected, desired))
        break;

      // locked, waiting for other threads' modification
      cpu_pause();
    }
  }

  /**
   * @brief update version
   * */
  void update_version() { code_.fetch_add(kVerAtom); }

  /**
   * @brief release exclusive lock and update version
   * @param update whether to update version, true by default
   * */
  void unlock_exclusive(bool update = true) {
    static_assert(kLockBit * 2 == kVerAtom);
    assert(code_.load(load_order) & kLockBit);
    if(update) code_.fetch_add(kLockBit); // unlock and update version
    else code_.fetch_sub(kLockBit);
  }

  /**
   * @brief if any writer holds the exclusive lock, wait until the writer
   * releasing the exclusive lock, then load the latest version
   * */
  uint64_t lock_shared() const {
    while(true) {
      uint64_t code = code_.load(load_order);
      if((code & kLockBit) == 0) {
        return code & kVerMask;
      }

      // locked, waiting for other threads' modification
      cpu_pause();
    }
  }

  /**
   * @brief if any writer holds the exclusive lock, wait until the writer
   * releasing the exclusive lock, then compare version with the latest;
   * if version has changed, restart your read operation
   * */
  bool unlock_shared(uint64_t version) const {
    uint64_t code = code_.load(load_order);

    // locked, waiting for other threads' modification
    while((code & kLockBit) != 0) {
      cpu_pause();
      code = code_.load(load_order);
    }

    // version has changed, retry
    if((code & kVerMask) != version)
      return false;

    // version does not change, unlock shared
    return true;
  }

  /**
   * @brief check if version changed, opposite to unlock_shared
   * */
  bool version_changed(uint64_t version) const {
    return !unlock_shared(version);
  }

  /**
   * @brief is locked
   * */
  bool locked() const { return code_.load(load_order) & kLockBit; }

  /**
   * @brief version lock atomic type, be careful when operating on this
   * */
  std::atomic<uint64_t>& atomic() { return code_; }

  /**
   * @brief reserved bits mask for customized design
   * */
  static constexpr uint64_t rest_mask() { return kRestMask; }
};

}

#endif //UTIL_LOCK_H
