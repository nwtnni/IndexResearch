/*
 * Copyright (c) 2024-Present, Chen Yuan <yuan.chen@whu.edu.cn>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */

#ifndef UTIL_TYPE_H
#define UTIL_TYPE_H

#include <cstdint>
#include <cassert>
#include <cstring>
#include "strutil.h"

namespace util {

/* customized value for KVPair specialization */
struct RowValue {};

/* variable length key */
struct String {
  int len;      // string length
  char str[];   // character array

  String() = delete;

  static void make_string(String* s, char* str, int len) {
    assert(s != nullptr && str != nullptr);
    s->len = len;
    memcpy(s->str, str, len);
  }

  static String* make_string(char* str, int len) {
    auto* ret = (String*) malloc(len + sizeof(String));
    make_string(ret, str, len);
    return ret;
  }

  bool operator<(const String& k) {
    return compare(str, len, (char*) k.str, k.len) < 0;
  }

  bool operator>(const String& k) {
    return compare(str, len, (char*) k.str, k.len) > 0;
  }

  bool operator==(const String& k) {
    if(len != k.len) return false;
    return !compare(str, len, (char*) k.str, k.len);
  }

  bool operator!=(const String& k) {
    if(len != k.len) return true;
    return compare(str, len, (char*) k.str, k.len);
  }
};

template<typename K, typename V>
struct KVPair {
  K key;
  V value;
};

template<typename K>
struct KVPair<K, RowValue> {
  K key;
  int vlen;
  char value[];

  KVPair() = delete;

  static void make_kv(KVPair* kv, const K& key, void* value, int vlen) {
    assert(kv != nullptr && value != nullptr);
    kv->key = key, kv->vlen = vlen;
    memcpy(kv->value, value, vlen);
  }

  static KVPair* make_kv(const K& key, void* value, int vlen){
    auto* kv = (KVPair*) malloc(sizeof(KVPair) + vlen);
    make_kv(kv, key, value, vlen);
    return kv;
  }
};

template<typename V>
struct KVPair<String, V> {
  V value;
  String key;

  KVPair() = delete;

  static void make_kv(KVPair* kv, char* key, int klen, const V& value) {
    assert(kv != nullptr && key != nullptr);
    new(&kv->value) V(value);
    String::make_string(&kv->key, key, klen);
  }

  static KVPair* make_kv(char* key, int klen, const V& value) {
    auto* kv = (KVPair*) malloc(sizeof(KVPair) + klen);
    make_kv(kv, key, klen, value);
    return kv;
  }

  static void make_kv(KVPair* kv, char* key, int klen, V&& value) {
    assert(kv != nullptr && key != nullptr);
    new(&kv->value) V(std::move(value));
    String::make_string(&kv->key, key, klen);
  }

  static KVPair* make_kv(char* key, int klen, V&& value) {
    auto* kv = (KVPair*) malloc(sizeof(KVPair) + klen);
    make_kv(kv, key, klen, std::move(value));
    return kv;
  }
};

template<>
struct KVPair<String, RowValue> {
  int vlen;
  union {
    String key;
    struct {
      int klen;
      char kv[];
    };
  };

  KVPair() = delete;

  static void make_kv(KVPair* kv, char* key, int klen, void* value, int vlen) {
    assert(kv != nullptr && key != nullptr && value != nullptr);
    kv->vlen = vlen, kv->klen = klen;
    memcpy(kv->kv, key, klen);
    memcpy(kv->kv + klen, value, vlen);
  }

  static KVPair* make_kv(char* key, int klen, void* value, int vlen) {
    auto* kv = (KVPair*) malloc(sizeof(KVPair) + klen + vlen);
    make_kv(kv, key, klen, value, vlen);
    return kv;
  }
};

}

#endif //UTIL_TYPE_H
