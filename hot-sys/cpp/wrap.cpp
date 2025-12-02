#include <memory>

#include "wrap.h"
#include "hot.h"

using InnerHotTree = HotTree<std::uint64_t, std::uint32_t>;

HOTTree::HOTTree()
    : inner_(new InnerHotTree())
{}

HOTTree::~HOTTree() {
    delete static_cast<InnerHotTree*>(inner_);
}

bool HOTTree::upsert(std::uint64_t key, std::uint32_t value) {
    return static_cast<InnerHotTree*>(inner_)->upsert(key, value);
}

bool HOTTree::search(std::uint64_t key, std::uint32_t* value) {
    return static_cast<InnerHotTree*>(inner_)->search(key, *value);
}

std::unique_ptr<HOTTree> hottree_new() {
    return std::make_unique<HOTTree>();
}

bool hottree_upsert(HOTTree* tree, std::uint64_t key, std::uint32_t value) {
    return tree->upsert(key, value);
}

bool hottree_search(HOTTree* tree, std::uint64_t key, std::uint32_t* value) {
    return tree->search(key, value);
}
