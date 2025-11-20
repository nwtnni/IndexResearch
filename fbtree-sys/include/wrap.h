#include <cstdint>
#include <memory>

#include "fbtree.h"

using FbInt = FeatureBTree::FBTree<uint64_t, uint64_t>;

std::unique_ptr<FbInt> fbtree_new();

void fbtree_upsert(FbInt *tree, uint64_t key, uint64_t value);

void fbtree_update(FbInt *tree, uint64_t key, uint64_t value);

bool fbtree_lookup(FbInt *tree, uint64_t key, uint64_t *value);
