#include <cstdint>
#include <memory>

#include "hot.h"

class HOTTree {
public:
	HotTree<uint64_t, uint32_t> inner;	
};

std::unique_ptr<HOTTree> hottree_new();

bool hottree_upsert(HOTTree *tree, uint64_t key, uint32_t val);

bool hottree_search(HOTTree *tree, uint64_t key, uint32_t *val);
