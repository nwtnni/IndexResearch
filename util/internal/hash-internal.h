#ifndef UTIL_HASH_INTERNAL_H
#define UTIL_HASH_INTERNAL_H

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <cstring>
#include <byteswap.h>

namespace util::internal {

namespace jenkins_internal {
// These implementations are based on hash functions (designed for 32-bit machines) by Bob Jenkins in https://burtleburtle.net/bob/index.html

// One-at-a-Time Hash in Jenkins's blog
inline uint64_t jenkins_byte_hash(void* key, size_t len) {
  uint64_t hash = 0;
  for(size_t i = 0; i < len; i++) {
    hash += ((char*) key)[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

inline uint32_t rot(uint32_t x, int k) {
  return (x << k) | (x >> (32 - k));
}

inline void mix(uint32_t& a, uint32_t& b, uint32_t& c) {
  a -= c, a ^= rot(c, 4), c += b;
  b -= a, b ^= rot(a, 6), a += c;
  c -= b, c ^= rot(b, 8), b += a;
  a -= c, a ^= rot(c, 16), c += b;
  b -= a, b ^= rot(a, 19), a += c;
  c -= b, c ^= rot(b, 4), b += a;
}

inline void final(uint32_t& a, uint32_t& b, uint32_t& c) {
  c ^= b, c -= rot(b, 14);
  a ^= c, a -= rot(c, 11);
  b ^= a, b -= rot(a, 25);
  c ^= b, c -= rot(b, 16);
  a ^= c, a -= rot(c, 4);
  b ^= a, b -= rot(a, 14);
  c ^= b, c -= rot(b, 24);
}

// based on lookup3.c by Bob Jenkins, https://burtleburtle.net/bob/c/lookup3.c
inline uint64_t jenkins_hash(void* key, size_t len) {
  uint64_t hash = 0xc6a4a7935bd1e995ull;; // seed used in murmurhash64
  uint32_t& first = ((uint32_t*) &hash)[0];
  uint32_t& second = ((uint32_t*) &hash)[1];
  uint32_t a, b, c = 0xdeadbeef + len + first;
  a = c, b = c, c += second;

  /* Jenkins's original implementation carefully deals with unaligned memory access this
   * may be because some early CPUs do not support unaligned memory access or have
   * significant performance penalty, but in my experience, unaligned memory access
   * has hardly any performance penalty on modern CPUs, even in SIMD instructions */
  const uint32_t* k = (const uint32_t*) key;
  while(len > 12) {
    a += k[0], b += k[1], c += k[2];
    mix(a, b, c), len -= 12, k += 3;
  }

  switch(len) {
    case 12:
      c += k[2], b += k[1], a += k[0];
      break;
    case 11:
      c += k[2] & 0xffffff, b += k[1], a += k[0];
      break;
    case 10:
      c += k[2] & 0xffff, b += k[1], a += k[0];
      break;
    case 9 :
      c += k[2] & 0xff, b += k[1], a += k[0];
      break;
    case 8 :
      b += k[1], a += k[0];
      break;
    case 7 :
      b += k[1] & 0xffffff, a += k[0];
      break;
    case 6 :
      b += k[1] & 0xffff, a += k[0];
      break;
    case 5 :
      b += k[1] & 0xff, a += k[0];
      break;
    case 4 :
      a += k[0];
      break;
    case 3 :
      a += k[0] & 0xffffff;
      break;
    case 2 :
      a += k[0] & 0xffff;
      break;
    case 1 :
      a += k[0] & 0xff;
      break;
    case 0 :
      first = c, second = b;
      return hash;  /* zero length strings require no mixing */
  }

  final(a, b, c);
  first = c, second = b;
  return hash;
}

inline uint64_t Rot64(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

inline void ShortMix(uint64_t& h0, uint64_t& h1, uint64_t& h2, uint64_t& h3) {
  h2 = Rot64(h2, 50), h2 += h3, h0 ^= h2;
  h3 = Rot64(h3, 52), h3 += h0, h1 ^= h3;
  h0 = Rot64(h0, 30), h0 += h1, h2 ^= h0;
  h1 = Rot64(h1, 41), h1 += h2, h3 ^= h1;
  h2 = Rot64(h2, 54), h2 += h3, h0 ^= h2;
  h3 = Rot64(h3, 48), h3 += h0, h1 ^= h3;
  h0 = Rot64(h0, 38), h0 += h1, h2 ^= h0;
  h1 = Rot64(h1, 37), h1 += h2, h3 ^= h1;
  h2 = Rot64(h2, 62), h2 += h3, h0 ^= h2;
  h3 = Rot64(h3, 34), h3 += h0, h1 ^= h3;
  h0 = Rot64(h0, 5), h0 += h1, h2 ^= h0;
  h1 = Rot64(h1, 36), h1 += h2, h3 ^= h1;
}

inline void ShortEnd(uint64_t& h0, uint64_t& h1, uint64_t& h2, uint64_t& h3) {
  h3 ^= h2, h2 = Rot64(h2, 15), h3 += h2;
  h0 ^= h3, h3 = Rot64(h3, 52), h0 += h3;
  h1 ^= h0, h0 = Rot64(h0, 26), h1 += h0;
  h2 ^= h1, h1 = Rot64(h1, 51), h2 += h1;
  h3 ^= h2, h2 = Rot64(h2, 28), h3 += h2;
  h0 ^= h3, h3 = Rot64(h3, 9), h0 += h3;
  h1 ^= h0, h0 = Rot64(h0, 47), h1 += h0;
  h2 ^= h1, h1 = Rot64(h1, 54), h2 += h1;
  h3 ^= h2, h2 = Rot64(h2, 32), h3 += h2;
  h0 ^= h3, h3 = Rot64(h3, 25), h0 += h3;
  h1 ^= h0, h0 = Rot64(h0, 63), h1 += h0;
}

inline void Short(const void* message, size_t len, uint64_t& hash1, uint64_t& hash2) {
  const uint64_t sc_const = 0xdeadbeefdeadbeefLL;
  union { const uint8_t* p8; uint32_t* p32; uint64_t* p64; } u;
  u.p8 = (const uint8_t*) message;
  size_t remainder = len % 32;
  uint64_t a = hash1, b = hash2;
  uint64_t c = sc_const, d = sc_const;

  if(len > 15) {
    const uint64_t* end = u.p64 + (len / 32) * 4;

    // handle all complete sets of 32 bytes
    for(; u.p64 < end; u.p64 += 4) {
      c += u.p64[0], d += u.p64[1];
      ShortMix(a, b, c, d);
      a += u.p64[2], b += u.p64[3];
    }

    //Handle the case of 16+ remaining bytes.
    if(remainder >= 16) {
      c += u.p64[0], d += u.p64[1];
      ShortMix(a, b, c, d);
      u.p64 += 2, remainder -= 16;
    }
  }

  // Handle the last 0..15 bytes, and its length
  d += ((uint64_t) len) << 56;
  switch(remainder) {
    case 15:
      d += ((uint64_t) u.p8[14]) << 48;
    case 14:
      d += ((uint64_t) u.p8[13]) << 40;
    case 13:
      d += ((uint64_t) u.p8[12]) << 32;
    case 12:
      d += u.p32[2], c += u.p64[0];
      break;
    case 11:
      d += ((uint64_t) u.p8[10]) << 16;
    case 10:
      d += ((uint64_t) u.p8[9]) << 8;
    case 9:
      d += (uint64_t) u.p8[8];
    case 8:
      c += u.p64[0];
      break;
    case 7:
      c += ((uint64_t) u.p8[6]) << 48;
    case 6:
      c += ((uint64_t) u.p8[5]) << 40;
    case 5:
      c += ((uint64_t) u.p8[4]) << 32;
    case 4:
      c += u.p32[0];
      break;
    case 3:
      c += ((uint64_t) u.p8[2]) << 16;
    case 2:
      c += ((uint64_t) u.p8[1]) << 8;
    case 1:
      c += (uint64_t) u.p8[0];
      break;
    case 0:
      c += sc_const, d += sc_const;
  }
  ShortEnd(a, b, c, d);
  hash1 = a, hash2 = b;
}

inline void Mix(const uint64_t* data,
                uint64_t& s0, uint64_t& s1, uint64_t& s2, uint64_t& s3,
                uint64_t& s4, uint64_t& s5, uint64_t& s6, uint64_t& s7,
                uint64_t& s8, uint64_t& s9, uint64_t& s10, uint64_t& s11) {
  s0 += data[0], s2 ^= s10, s11 ^= s0, s0 = Rot64(s0, 11), s11 += s1;
  s1 += data[1], s3 ^= s11, s0 ^= s1, s1 = Rot64(s1, 32), s0 += s2;
  s2 += data[2], s4 ^= s0, s1 ^= s2, s2 = Rot64(s2, 43), s1 += s3;
  s3 += data[3], s5 ^= s1, s2 ^= s3, s3 = Rot64(s3, 31), s2 += s4;
  s4 += data[4], s6 ^= s2, s3 ^= s4, s4 = Rot64(s4, 17), s3 += s5;
  s5 += data[5], s7 ^= s3, s4 ^= s5, s5 = Rot64(s5, 28), s4 += s6;
  s6 += data[6], s8 ^= s4, s5 ^= s6, s6 = Rot64(s6, 39), s5 += s7;
  s7 += data[7], s9 ^= s5, s6 ^= s7, s7 = Rot64(s7, 57), s6 += s8;
  s8 += data[8], s10 ^= s6, s7 ^= s8, s8 = Rot64(s8, 55), s7 += s9;
  s9 += data[9], s11 ^= s7, s8 ^= s9, s9 = Rot64(s9, 54), s8 += s10;
  s10 += data[10], s0 ^= s8, s9 ^= s10, s10 = Rot64(s10, 22), s9 += s11;
  s11 += data[11], s1 ^= s9, s10 ^= s11, s11 = Rot64(s11, 46), s10 += s0;
}

inline void EndPartial(uint64_t& h0, uint64_t& h1, uint64_t& h2, uint64_t& h3,
                       uint64_t& h4, uint64_t& h5, uint64_t& h6, uint64_t& h7,
                       uint64_t& h8, uint64_t& h9, uint64_t& h10, uint64_t& h11) {
  h11 += h1, h2 ^= h11, h1 = Rot64(h1, 44);
  h0 += h2, h3 ^= h0, h2 = Rot64(h2, 15);
  h1 += h3, h4 ^= h1, h3 = Rot64(h3, 34);
  h2 += h4, h5 ^= h2, h4 = Rot64(h4, 21);
  h3 += h5, h6 ^= h3, h5 = Rot64(h5, 38);
  h4 += h6, h7 ^= h4, h6 = Rot64(h6, 33);
  h5 += h7, h8 ^= h5, h7 = Rot64(h7, 10);
  h6 += h8, h9 ^= h6, h8 = Rot64(h8, 13);
  h7 += h9, h10 ^= h7, h9 = Rot64(h9, 38);
  h8 += h10, h11 ^= h8, h10 = Rot64(h10, 53);
  h9 += h11, h0 ^= h9, h11 = Rot64(h11, 42);
  h10 += h0, h1 ^= h10, h0 = Rot64(h0, 54);
}

inline void End(const uint64_t* data,
                uint64_t& h0, uint64_t& h1, uint64_t& h2, uint64_t& h3,
                uint64_t& h4, uint64_t& h5, uint64_t& h6, uint64_t& h7,
                uint64_t& h8, uint64_t& h9, uint64_t& h10, uint64_t& h11) {
  h0 += data[0], h1 += data[1], h2 += data[2], h3 += data[3];
  h4 += data[4], h5 += data[5], h6 += data[6], h7 += data[7];
  h8 += data[8], h9 += data[9], h10 += data[10], h11 += data[11];
  EndPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
  EndPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
  EndPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
}

// A faster hash function on 64-bit machines by Bob Jenkins named spooky v2, https://burtleburtle.net/bob/hash/spooky.html
inline std::pair<uint64_t, uint64_t> spooky_hash(void* key, size_t len) {
  constexpr uint64_t sc_const = 0xdeadbeefdeadbeefLL;
  constexpr size_t sc_numVars = 12, sc_blockSize = sc_numVars * 8;
  constexpr size_t sc_bufSize = 2 * sc_blockSize;

  std::pair<uint64_t, uint64_t> hash;
  // seed used in google city hash
  hash.first = 0xc3a5c85c97cb3127ULL;
  hash.second = 0xb492b66fbe98f273ULL;
  if(len < sc_bufSize) {
    Short(key, len, hash.first, hash.second);
    return hash;
  }

  uint64_t h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
  uint64_t buf[sc_numVars];
  uint64_t* end;
  union { const uint8_t* p8;uint64_t* p64;size_t i; } u;
  size_t remainder;
  h0 = h3 = h6 = h9 = hash.first;
  h1 = h4 = h7 = h10 = hash.second;
  h2 = h5 = h8 = h11 = sc_const;

  u.p8 = (const uint8_t*) key;
  end = u.p64 + (len / sc_blockSize) * sc_numVars;

  // handle all whole sc_blockSize blocks of bytes
  while(u.p64 < end) {
    Mix(u.p64, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
    u.p64 += sc_numVars;
  }

  // handle the last partial block of sc_blockSize bytes
  remainder = (len - ((const uint8_t*) end - (const uint8_t*) key));
  memcpy(buf, end, remainder);
  memset(((uint8_t*) buf) + remainder, 0, sc_blockSize - remainder);
  ((uint8_t*) buf)[sc_blockSize - 1] = remainder;

  // do some final mixing
  End(buf, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
  hash.first = h0, hash.second = h1;
  return hash;
}

}

namespace murmur_internal {
// I forget where I get this code, a similar publicly available implementation can be found in https://github.com/rurban/smhasher.git
inline size_t murmur_hash32(void* key, size_t len) {
  const unsigned int seed = 0xc70f6907UL;
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  unsigned int h = seed ^ len;
  auto data = (const unsigned char*) key;

  while(len >= 4) {
    unsigned int k = *(unsigned int*) data;
    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;
    data += 4;
    len -= 4;
  }

  switch(len) {
    case 3:
      h ^= data[2] << 16;
    case 2:
      h ^= data[1] << 8;
    case 1:
      h ^= data[0];
      h *= m;
  }

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

inline size_t murmur_hash64(void* key, size_t len) {
  const uint64_t m = 0xc6a4a7935bd1e995ull;
  const std::size_t r = 47;
  uint64_t seed = 7079;

  uint64_t h = seed ^ (len * m);

  auto data = (const uint64_t*) key;
  const uint64_t* end = data + (len / 8);

  while(data != end) {
    uint64_t k = *data++;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  auto data2 = (const unsigned char*) data;

  switch(len & 7ull) {
    case 7:
      h ^= uint64_t(data2[6]) << 48ull;
    case 6:
      h ^= uint64_t(data2[5]) << 40ull;
    case 5:
      h ^= uint64_t(data2[4]) << 32ull;
    case 4:
      h ^= uint64_t(data2[3]) << 24ull;
    case 3:
      h ^= uint64_t(data2[2]) << 16ull;
    case 2:
      h ^= uint64_t(data2[1]) << 8ull;
    case 1:
      h ^= uint64_t(data2[0]);
      h *= m;
  }

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

}

namespace faster_internal {
// Copyright (c) Microsoft Corporation. Licensed under the MIT License.
// See details in https://github.com/microsoft/FASTER.git
inline uint64_t Rotr64(uint64_t x, size_t n) {
  return (((x) >> n) | ((x) << (64 - n)));
}

inline size_t faster_hash(void* key, size_t len) {
  // 40343 is a "magic constant" that works well,
  // 38299 is another good value.
  // Both are primes and have a good distribution of bits.
  const uint64_t kMagicNum = 40343;
  uint64_t hashState = len;
  const char* data = (const char*) key;

  for(size_t idx = 0; idx < len; ++idx) {
    hashState = kMagicNum * hashState + data[idx];
  }

  // The final scrambling helps with short keys that vary only on the high order bits.
  // Low order bits are not always well distributed so shift them to the high end, where they'll
  // form part of the 14-bit tag.
  return Rotr64(kMagicNum * hashState, 6);
}

inline size_t faster_hash(uint64_t key) {
  uint64_t local_rand = key;
  uint64_t local_rand_hash = 8;
  local_rand_hash = 40343 * local_rand_hash + ((local_rand) & 0xFFFF);
  local_rand_hash = 40343 * local_rand_hash + ((local_rand >> 16) & 0xFFFF);
  local_rand_hash = 40343 * local_rand_hash + ((local_rand >> 32) & 0xFFFF);
  local_rand_hash = 40343 * local_rand_hash + (local_rand >> 48);
  local_rand_hash = 40343 * local_rand_hash;
  return Rotr64(local_rand_hash, 43);
}

}

namespace city_internal {
// Copyright (c) 2011 Google, Inc. Licensed under the MIT License.
// See details in https://github.com/google/cityhash.git
typedef uint8_t uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef std::pair<uint64, uint64> uint128;

// Some primes between 2^63 and 2^64 for various uses.
constexpr uint64 k0 = 0xc3a5c85c97cb3127ULL;
constexpr uint64 k1 = 0xb492b66fbe98f273ULL;
constexpr uint64 k2 = 0x9ae16a3b2f90404fULL;

inline uint64 Uint128Low64(const uint128& x) { return x.first; }

inline uint64 Uint128High64(const uint128& x) { return x.second; }

// Hash 128 input bits down to 64 bits of output.
// This is intended to be a reasonably good hash function.
inline uint64 Hash128to64(const uint128& x) {
  // Murmur-inspired hashing.
  const uint64 kMul = 0x9ddfea08eb382d69ULL;
  uint64 a = (Uint128Low64(x) ^ Uint128High64(x)) * kMul;
  a ^= (a >> 47);
  uint64 b = (Uint128High64(x) ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

inline uint64 Fetch64(const char* p) {
  uint64 result;
  memcpy(&result, p, sizeof(result));
  return result;
}

inline uint32 Fetch32(const char* p) {
  uint32 result;
  memcpy(&result, p, sizeof(result));
  return result;
}

// Bitwise right rotate.  Normally this will compile to a single
// instruction, especially if the shift is a manifest constant.
inline uint64 Rotate(uint64 val, int shift) {
  // Avoid shifting by 64: doing so yields an undefined result.
  return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

inline uint64 ShiftMix(uint64 val) {
  return val ^ (val >> 47);
}

inline uint64 HashLen16(uint64 u, uint64 v) {
  return Hash128to64(uint128(u, v));
}

inline uint64 HashLen16(uint64 u, uint64 v, uint64 mul) {
  // Murmur-inspired hashing.
  uint64 a = (u ^ v) * mul;
  a ^= (a >> 47);
  uint64 b = (v ^ a) * mul;
  b ^= (b >> 47);
  b *= mul;
  return b;
}

inline uint64 HashLen0to16(const char* s, size_t len) {
  if(len >= 8) {
    uint64 mul = k2 + len * 2;
    uint64 a = Fetch64(s) + k2;
    uint64 b = Fetch64(s + len - 8);
    uint64 c = Rotate(b, 37) * mul + a;
    uint64 d = (Rotate(a, 25) + b) * mul;
    return HashLen16(c, d, mul);
  }
  if(len >= 4) {
    uint64 mul = k2 + len * 2;
    uint64 a = Fetch32(s);
    return HashLen16(len + (a << 3), Fetch32(s + len - 4), mul);
  }
  if(len > 0) {
    uint8 a = static_cast<uint8>(s[0]);
    uint8 b = static_cast<uint8>(s[len >> 1]);
    uint8 c = static_cast<uint8>(s[len - 1]);
    uint32 y = static_cast<uint32>(a) + (static_cast<uint32>(b) << 8);
    uint32 z = static_cast<uint32>(len) + (static_cast<uint32>(c) << 2);
    return ShiftMix(y * k2 ^ z * k0) * k2;
  }
  return k2;
}

// This probably works well for 16-byte strings as well, but it may be overkill
// in that case.
inline uint64 HashLen17to32(const char* s, size_t len) {
  uint64 mul = k2 + len * 2;
  uint64 a = Fetch64(s) * k1;
  uint64 b = Fetch64(s + 8);
  uint64 c = Fetch64(s + len - 8) * mul;
  uint64 d = Fetch64(s + len - 16) * k2;
  return HashLen16(Rotate(a + b, 43) + Rotate(c, 30) + d,
                   a + Rotate(b + k2, 18) + c, mul);
}

// Return an 8-byte hash for 33 to 64 bytes.
inline uint64 HashLen33to64(const char* s, size_t len) {
  uint64 mul = k2 + len * 2;
  uint64 a = Fetch64(s) * k2;
  uint64 b = Fetch64(s + 8);
  uint64 c = Fetch64(s + len - 24);
  uint64 d = Fetch64(s + len - 32);
  uint64 e = Fetch64(s + 16) * k2;
  uint64 f = Fetch64(s + 24) * 9;
  uint64 g = Fetch64(s + len - 8);
  uint64 h = Fetch64(s + len - 16) * mul;
  uint64 u = Rotate(a + g, 43) + (Rotate(b, 30) + c) * 9;
  uint64 v = ((a + g) ^ d) + f + 1;
  uint64 w = bswap_64((u + v) * mul) + h;
  uint64 x = Rotate(e + f, 42) + c;
  uint64 y = (bswap_64((v + w) * mul) + g) * mul;
  uint64 z = e + f + c;
  a = bswap_64((x + z) * mul + y) + b;
  b = ShiftMix((z + a) * mul + d + h) * mul;
  return b + x;
}

// Return a 16-byte hash for 48 bytes.  Quick and dirty.
// Callers do best to use "random-looking" values for a and b.
inline std::pair<uint64, uint64> WeakHashLen32WithSeeds(
  uint64 w, uint64 x, uint64 y, uint64 z, uint64 a, uint64 b) {
  a += w;
  b = Rotate(b + a + z, 21);
  uint64 c = a;
  a += x;
  a += y;
  b += Rotate(a, 44);
  return std::make_pair(a + z, b + c);
}

// Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
inline std::pair<uint64, uint64> WeakHashLen32WithSeeds(
  const char* s, uint64 a, uint64 b) {
  return WeakHashLen32WithSeeds(Fetch64(s),
                                Fetch64(s + 8),
                                Fetch64(s + 16),
                                Fetch64(s + 24),
                                a,
                                b);
}

// A subroutine for CityHash128().  Returns a decent 128-bit hash for strings
// of any length representable in signed long.  Based on City and Murmur.
static uint128 CityMurmur(const char* s, size_t len, uint128 seed) {
  uint64 a = Uint128Low64(seed);
  uint64 b = Uint128High64(seed);
  uint64 c = 0;
  uint64 d = 0;
  if(len <= 16) {
    a = ShiftMix(a * k1) * k1;
    c = b * k1 + HashLen0to16(s, len);
    d = ShiftMix(a + (len >= 8 ? Fetch64(s) : c));
  } else {
    c = HashLen16(Fetch64(s + len - 8) + k1, a);
    d = HashLen16(b + len, c + Fetch64(s + len - 16));
    a += d;
    // len > 16 here, so do...while is safe
    do {
      a ^= ShiftMix(Fetch64(s) * k1) * k1;
      a *= k1;
      b ^= a;
      c ^= ShiftMix(Fetch64(s + 8) * k1) * k1;
      c *= k1;
      d ^= c;
      s += 16;
      len -= 16;
    } while(len > 16);
  }
  a = HashLen16(a, c);
  b = HashLen16(d, b);
  return uint128(a ^ b, HashLen16(b, a));
}

inline long city_likely(long exp) { return __builtin_expect(exp, 1); }

inline uint128 CityHash128WithSeed(const char* s, size_t len, uint128 seed) {
  if(len < 128) {
    return CityMurmur(s, len, seed);
  }

  // We expect len >= 128 to be the common case.  Keep 56 bytes of state:
  // v, w, x, y, and z.
  std::pair<uint64, uint64> v, w;
  uint64 x = Uint128Low64(seed);
  uint64 y = Uint128High64(seed);
  uint64 z = len * k1;
  v.first = Rotate(y ^ k1, 49) * k1 + Fetch64(s);
  v.second = Rotate(v.first, 42) * k1 + Fetch64(s + 8);
  w.first = Rotate(y + z, 35) * k1 + x;
  w.second = Rotate(x + Fetch64(s + 88), 53) * k1;

  // This is the same inner loop as CityHash64(), manually unrolled.
  do {
    x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
    y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
    x ^= w.second;
    y += v.first + Fetch64(s + 40);
    z = Rotate(z + w.first, 33) * k1;
    v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
    w = WeakHashLen32WithSeeds(s + 32, z + w.second, y + Fetch64(s + 16));
    std::swap(z, x);
    s += 64;
    x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
    y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
    x ^= w.second;
    y += v.first + Fetch64(s + 40);
    z = Rotate(z + w.first, 33) * k1;
    v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
    w = WeakHashLen32WithSeeds(s + 32, z + w.second, y + Fetch64(s + 16));
    std::swap(z, x);
    s += 64;
    len -= 128;
  } while(city_likely(len >= 128));
  x += Rotate(v.first + z, 49) * k0;
  y = y * k0 + Rotate(w.second, 37);
  z = z * k0 + Rotate(w.first, 27);
  w.first *= 9;
  v.first *= k0;
  // If 0 < len < 128, hash up to 4 chunks of 32 bytes each from the end of s.
  for(size_t tail_done = 0; tail_done < len;) {
    tail_done += 32;
    y = Rotate(x + y, 42) * k0 + v.second;
    w.first += Fetch64(s + len - tail_done + 16);
    x = x * k0 + w.first;
    z += w.second + Fetch64(s + len - tail_done);
    w.second += v.first;
    v = WeakHashLen32WithSeeds(s + len - tail_done, v.first + z, v.second);
    v.first *= k0;
  }
  // At this point our 56 bytes of state should contain more than
  // enough information for a strong 128-bit hash.  We use two
  // different 56-byte-to-8-byte hashes to get a 16-byte final result.
  x = HashLen16(x, v.first);
  y = HashLen16(y + z, w.first);
  return uint128(HashLen16(x + v.second, w.second) + y,
                 HashLen16(x + w.second, y + v.second));
}

inline size_t city_hash64(const char* s, size_t len) {
  if(len <= 32) {
    if(len <= 16) {
      return HashLen0to16(s, len);
    } else {
      return HashLen17to32(s, len);
    }
  } else if(len <= 64) {
    return HashLen33to64(s, len);
  }

  // For strings over 64 bytes we hash the end first, and then as we
  // loop we keep 56 bytes of state: v, w, x, y, and z.
  uint64 x = Fetch64(s + len - 40);
  uint64 y = Fetch64(s + len - 16) + Fetch64(s + len - 56);
  uint64 z = HashLen16(Fetch64(s + len - 48) + len, Fetch64(s + len - 24));
  std::pair<uint64, uint64> v = WeakHashLen32WithSeeds(s + len - 64, len, z);
  std::pair<uint64, uint64> w = WeakHashLen32WithSeeds(s + len - 32, y + k1, x);
  x = x * k1 + Fetch64(s);

  // Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
  len = (len - 1) & ~static_cast<size_t>(63);
  do {
    x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
    y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
    x ^= w.second;
    y += v.first + Fetch64(s + 40);
    z = Rotate(z + w.first, 33) * k1;
    v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
    w = WeakHashLen32WithSeeds(s + 32, z + w.second, y + Fetch64(s + 16));
    std::swap(z, x);
    s += 64;
    len -= 64;
  } while(len != 0);
  return HashLen16(HashLen16(v.first, w.first) + ShiftMix(y) * k1 + z,
                   HashLen16(v.second, w.second) + x);
}

inline uint128 city_hash128(const char* s, size_t len) {
  return len >= 16 ?
         CityHash128WithSeed(s + 16, len - 16,
                             uint128(Fetch64(s), Fetch64(s + 8) + k0)) :
         CityHash128WithSeed(s, len, uint128(k0, k1));
}

}

using jenkins_internal::jenkins_byte_hash;
using jenkins_internal::jenkins_hash;
using jenkins_internal::spooky_hash;

using murmur_internal::murmur_hash32;
using murmur_internal::murmur_hash64;

using faster_internal::faster_hash;

using city_internal::city_hash64;
using city_internal::city_hash128;

}

#endif //UTIL_HASH_INTERNAL_H
