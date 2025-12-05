#include <memory>

#include "wrap.h"
#include "hot.h"

using InnerHotTree_u64 = HotTree<std::uint64_t, std::uint64_t>;
using InnerHotTree_str  = HotTree<std::string, std::uint64_t>;

// hottree u64

HOTTreeU64::HOTTreeU64()
    : inner_(new InnerHotTree_u64())
{}

HOTTreeU64::~HOTTreeU64() {
    delete static_cast<InnerHotTree_u64*>(inner_);
}

bool HOTTreeU64::upsert_u64(std::uint64_t key, std::uint64_t value) {
    return static_cast<InnerHotTree_u64*>(inner_)->upsert(key, value);
}

bool HOTTreeU64::search_u64(std::uint64_t key, std::uint64_t* value) {
    return static_cast<InnerHotTree_u64*>(inner_)->search(key, *value);
}

std::unique_ptr<HOTTreeU64> hottree_u64_new() {
    return std::make_unique<HOTTreeU64>();
}

bool hottree_u64_upsert(HOTTreeU64* tree, std::uint64_t key, std::uint64_t value) {
    return tree->upsert_u64(key, value);
}

bool hottree_u64_search(HOTTreeU64* tree, std::uint64_t key, std::uint64_t* value) {
    return tree->search_u64(key, value);
}

// hottree string

HOTTreeString::HOTTreeString()
    : inner_(new InnerHotTree_str())
{}

HOTTreeString::~HOTTreeString() {
    delete static_cast<InnerHotTree_str*>(inner_);
}

bool HOTTreeString::upsert_str(const char* key, std::size_t keylen, std::uint64_t value) {
    return static_cast<InnerHotTree_str*>(inner_)
        ->upsert(const_cast<char*>(key), static_cast<int>(keylen), value);
}

bool HOTTreeString::search_str(const char* key, std::uint64_t* value) {
    return static_cast<InnerHotTree_str*>(inner_)
        ->search(const_cast<char*>(key), *value);
}

std::unique_ptr<HOTTreeString> hottree_string_new() {
    return std::make_unique<HOTTreeString>();
}

bool hottree_string_upsert(
    HOTTreeString* tree,
    const char* key,
    std::size_t keylen,
    std::uint64_t value
) {
    return tree->upsert_str(key, keylen, value);
}

bool hottree_string_search(
    HOTTreeString* tree,
    const char* key,
    std::uint64_t* value
) {
    return tree->search_str(key, value);
}
