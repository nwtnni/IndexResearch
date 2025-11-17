#include <memory>

#include "wrap.h"

using util::EpochGuard;

std::unique_ptr<FBTree> fbtree_new() { return std::make_unique<FBTree>(); }

void fbtree_upsert(FBTree *tree, uint64_t key, uint32_t value) {
  EpochGuard epoch_guard(tree->inner.get_epoch());
  tree->inner.upsert(key, value);
}

void fbtree_update(FBTree *tree, uint64_t key, uint32_t value) {
  EpochGuard epoch_guard(tree->inner.get_epoch());
  tree->inner.update(key, value);
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
