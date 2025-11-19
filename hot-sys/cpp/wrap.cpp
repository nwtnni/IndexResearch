#include <memory>

#include "wrap.h"

std::unique_ptr<HOTTree> hottree_new() { return std::make_unique<HOTTree>(); }

bool hottree_upsert(HOTTree *tree, uint64_t key, uint32_t value) {
	return tree->inner.upsert(key, value);
}

bool hottree_search(HOTTree *tree, uint64_t key, uint32_t *value) {
	return tree->inner.search(key, value);
}
