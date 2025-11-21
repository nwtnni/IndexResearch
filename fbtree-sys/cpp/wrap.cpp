#include "wrap.h"

std::unique_ptr<FbU64> fbtree_u64_new() { return std::make_unique<FbU64>(); }

void fbtree_u64_upsert(FbU64 *tree, uint64_t key, uint64_t value) {
  EpochGuard epoch_guard(tree->get_epoch());
  void *old = tree->upsert(key, value);
  epoch_guard.retire(old);
}

void fbtree_u64_update(FbU64 *tree, uint64_t key, uint64_t value) {
  EpochGuard epoch_guard(tree->get_epoch());
  void *old = tree->update(key, value);
  epoch_guard.retire(old);
}

bool fbtree_u64_lookup(FbU64 *tree, uint64_t key, uint64_t *value) {
  EpochGuard epoch_guard(tree->get_epoch());
  auto pair = tree->lookup(key);
  if (pair == nullptr) {
    return false;
  }
  *value = pair->value;
  return true;
}

std::unique_ptr<FbU64Iter> fbtree_u64_iter(FbU64 *tree, uint64_t key) {
  auto iter = std::make_unique<FbU64Iter>(tree->get_epoch());
  iter->iter = tree->lower_bound(key);
  return iter;
}

void fbtree_u64_iter_advance(FbU64Iter *iter) { iter->iter.advance(); }

bool fbtree_u64_iter_end(FbU64Iter *iter) { return iter->iter.end(); }

uint64_t fbtree_u64_iter_get(FbU64Iter *iter) { return iter->iter->value; }

std::unique_ptr<FbString> fbtree_string_new() {
  return std::make_unique<FbString>();
}

void fbtree_string_upsert(FbString *tree, char *key, size_t keylen,
                          uint64_t value) {
  EpochGuard epoch_guard(tree->get_epoch());
  void *old = tree->upsert(key, keylen, value);
  epoch_guard.retire(old);
}

void fbtree_string_update(FbString *tree, char *key, size_t keylen,
                          uint64_t value) {
  EpochGuard epoch_guard(tree->get_epoch());
  void *old = tree->update(key, keylen, value);
  epoch_guard.retire(old);
}

bool fbtree_string_lookup(FbString *tree, char *key, size_t keylen,
                          uint64_t *value) {
  EpochGuard epoch_guard(tree->get_epoch());
  auto pair = tree->lookup(key, keylen);
  if (pair == nullptr) {
    return false;
  }
  *value = pair->value;
  return true;
}
