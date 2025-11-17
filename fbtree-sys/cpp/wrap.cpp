#include <memory>

#include "wrap.h"

using util::EpochGuard;

std::unique_ptr<FBTree> fbtree_new() { return std::make_unique<FBTree>(); }

void fbtree_upsert(FBTree *tree, uint64_t key, uint32_t value) {
  EpochGuard epoch_guard(tree->inner.get_epoch());
  void *old = tree->inner.upsert(key, value);
  epoch_guard.retire(old);
}

void fbtree_update(FBTree *tree, uint64_t key, uint32_t value) {
  EpochGuard epoch_guard(tree->inner.get_epoch());
  void *old = tree->inner.update(key, value);
  epoch_guard.retire(old);
}

bool fbtree_lookup(FBTree *tree, uint64_t key, uint32_t *value) {
  EpochGuard epoch_guard(tree->inner.get_epoch());
  auto pair = tree->inner.lookup(key);
  if (pair == nullptr) {
    return false;
  }
  *value = pair->value;
  return true;
}
