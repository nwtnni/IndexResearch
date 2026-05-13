/*
 * Copyright (c) 2025-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_POINTER_H
#define UTIL_POINTER_H

#include <cstddef>
#include <cstdint>
#include <atomic>

namespace util {

/**
 * @brief dense pointer that only uses the lower 48 bits for addressing/pointer,
 * the higher 16 bits are reserved, e.g. as a tag in hash table
 * */
class DensePointer {
  union {
    struct {
      uint64_t ptr_: 48;
      uint64_t res_: 16;
    };
    uint64_t code_;
  };

  friend class AtomicDense;

 public:
  DensePointer() : code_((uint64_t) nullptr) {}

  DensePointer(uint64_t code) : code_(code) {}

  DensePointer(void* ptr, uint16_t res) : ptr_((uint64_t) ptr), res_(res) {}

  DensePointer(const DensePointer& pointer) : code_(pointer.code_) {}

  ~DensePointer() = default;

  DensePointer& operator=(const DensePointer& pointer) {
    code_ = pointer.code_;
    return *this;
  }

  bool operator==(const DensePointer& pointer) const {
    return code_ == pointer.code_;
  }

  bool operator!=(const DensePointer& pointer) const {
    return code_ != pointer.code_;
  }

  void* pointer() const { return reinterpret_cast<void*>(ptr_); }

  uint16_t remain() const { return (uint16_t) res_; }
};

static_assert(sizeof(DensePointer) == 8);


/**
 * @brief atomic dense pointer, use in conjunction with dense pointer
 * */
class AtomicDense {
  std::atomic<uint64_t> code_;

 public:
  AtomicDense() : code_((uint64_t) nullptr) {}

  AtomicDense(const DensePointer& pointer) : code_(pointer.code_) {}

  AtomicDense(const AtomicDense&) = delete;

  AtomicDense& operator=(const AtomicDense&) = delete;

  DensePointer load(std::memory_order order = std::memory_order_seq_cst) const {
    return DensePointer(code_.load(order));
  }

  void store(DensePointer desired, std::memory_order order = std::memory_order_seq_cst) {
    code_.store(desired.code_, order);
  }

  DensePointer exchange(DensePointer desired, std::memory_order order = std::memory_order_seq_cst) {
    return code_.exchange(desired.code_, order);
  }

  bool compare_exchange_strong(DensePointer& expected, DensePointer desired,
                               std::memory_order order = std::memory_order_seq_cst) {
    return code_.compare_exchange_strong(expected.code_, desired.code_, order);
  }

  bool compare_exchange_strong(DensePointer& expected, DensePointer desired,
                               std::memory_order success, std::memory_order failure) {
    return code_.compare_exchange_strong(expected.code_, desired.code_, success, failure);
  }
};

}

#endif //UTIL_POINTER_H
