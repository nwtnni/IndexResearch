#include <cstdint>
#include <memory>

#include "fbtree.h"

using util::Epoch;
using util::EpochGuard;
using FbU64 = FeatureBTree::FBTree<uint64_t, uint64_t>;

template <typename K> class FbIter {
public:
  EpochGuard guard;
  typename FeatureBTree::FBTree<K, uint64_t>::iterator iter;
  FbIter(Epoch &epoch) : guard(epoch) {}
};

using FbU64Iter = FbIter<uint64_t>;

std::unique_ptr<FbU64> fbtree_u64_new();

void fbtree_u64_upsert(FbU64 *tree, uint64_t key, uint64_t value);

void fbtree_u64_update(FbU64 *tree, uint64_t key, uint64_t value);

bool fbtree_u64_lookup(FbU64 *tree, uint64_t key, uint64_t *value);

std::unique_ptr<FbU64Iter> fbtree_u64_iter(FbU64 *tree, uint64_t key);

void fbtree_u64_iter_advance(FbU64Iter *iter);

bool fbtree_u64_iter_end(FbU64Iter *iter);

uint64_t fbtree_u64_iter_get(FbU64Iter *iter);

using FbString = FeatureBTree::FBTree<FeatureBTree::String, uint64_t>;

std::unique_ptr<FbString> fbtree_string_new();

void fbtree_string_upsert(FbString *tree, char *key, const size_t keylen,
                          uint64_t value);

void fbtree_string_update(FbString *tree, char *key, const size_t keylen,
                          uint64_t value);

bool fbtree_string_lookup(FbString *tree, char *key, const size_t keylen,
                          uint64_t *value);

using FbStringIter = FbIter<FeatureBTree::String>;

std::unique_ptr<FbStringIter> fbtree_string_iter(FbString *tree, char *key,
                                                 const size_t keylen);

void fbtree_string_iter_advance(FbStringIter *iter);

bool fbtree_string_iter_end(FbStringIter *iter);

uint64_t fbtree_string_iter_get(FbStringIter *iter);
