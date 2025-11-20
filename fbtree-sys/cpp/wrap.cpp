#include "wrap.h"

using util::EpochGuard;

std::unique_ptr<FbInt> fbtree_new() { return std::make_unique<FbInt>(); }

void fbtree_upsert(FbInt *tree, uint64_t key, uint64_t value) {
  EpochGuard epoch_guard(tree->get_epoch());
  void *old = tree->upsert(key, value);
  epoch_guard.retire(old);
}

void fbtree_update(FbInt *tree, uint64_t key, uint64_t value) {
  EpochGuard epoch_guard(tree->get_epoch());
  void *old = tree->update(key, value);
  epoch_guard.retire(old);
}

bool fbtree_lookup(FbInt *tree, uint64_t key, uint64_t *value) {
  EpochGuard epoch_guard(tree->get_epoch());
  auto pair = tree->lookup(key);
  if (pair == nullptr) {
    return false;
  }
  *value = pair->value;
  return true;
}
