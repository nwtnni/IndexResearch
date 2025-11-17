#include <cstdint>
#include <memory>

#include "fbtree.h"

class FBTree {
public:
  FeatureBTree::FBTree<uint64_t, uint32_t> inner;
};

std::unique_ptr<FBTree> fbtree_new();

void fbtree_upsert(FBTree *tree, uint64_t key, uint32_t value);

void fbtree_update(FBTree *tree, uint64_t key, uint32_t value);

bool fbtree_lookup(FBTree *tree, uint64_t key, uint32_t *value);
