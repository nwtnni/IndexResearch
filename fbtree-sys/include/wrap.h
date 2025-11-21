#include <cstdint>
#include <memory>

#include "fbtree.h"

using FbU64 = FeatureBTree::FBTree<uint64_t, uint64_t>;

std::unique_ptr<FbU64> fbtree_u64_new();

void fbtree_u64_upsert(FbU64 *tree, uint64_t key, uint64_t value);

void fbtree_u64_update(FbU64 *tree, uint64_t key, uint64_t value);

bool fbtree_u64_lookup(FbU64 *tree, uint64_t key, uint64_t *value);

using FbString = FeatureBTree::FBTree<FeatureBTree::String, uint64_t>;

std::unique_ptr<FbString> fbtree_string_new();

void fbtree_string_upsert(FbString *tree, char *key, const size_t keylen,
                          uint64_t value);

void fbtree_string_update(FbString *tree, char *key, const size_t keylen,
                          uint64_t value);

bool fbtree_string_lookup(FbString *tree, char *key, const size_t keylen,
                          uint64_t *value);
