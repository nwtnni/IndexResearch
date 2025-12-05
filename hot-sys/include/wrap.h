#include <cstdint>
#include <memory>

// hottree u64
class HOTTreeU64 {
public:
    HOTTreeU64();
    ~HOTTreeU64();

    HOTTreeU64(const HOTTreeU64&) = delete;
    HOTTreeU64& operator=(const HOTTreeU64&) = delete;
    HOTTreeU64(HOTTreeU64&&) = delete;
    HOTTreeU64& operator=(HOTTreeU64&&) = delete;

    bool upsert_u64(std::uint64_t key, std::uint64_t value);
    bool search_u64(std::uint64_t key, std::uint64_t* value);

private:
    void* inner_; // pointer to real HotTree<uint64_t, uint64_t>
};

std::unique_ptr<HOTTreeU64> hottree_u64_new();

bool hottree_u64_upsert(HOTTreeU64 *tree, uint64_t key, uint64_t val);

bool hottree_u64_search(HOTTreeU64 *tree, uint64_t key, uint64_t *val);

// hottree string
class HOTTreeString {
public:
    HOTTreeString();
    ~HOTTreeString();

    HOTTreeString(const HOTTreeString&) = delete;
    HOTTreeString& operator=(const HOTTreeString&) = delete;
    HOTTreeString(HOTTreeString&&) = delete;
    HOTTreeString& operator=(HOTTreeString&&) = delete;

    bool upsert_str(const char* key, std::size_t keylen, std::uint64_t value);
    bool search_str(const char* key, std::uint64_t* value);

private:
    void* inner_;
};

std::unique_ptr<HOTTreeString> hottree_string_new();

bool hottree_string_upsert(
    HOTTreeString* tree,
    const char* key,
    std::size_t keylen,
    std::uint64_t value
);

bool hottree_string_search(
    HOTTreeString* tree,
    const char* key,
    std::uint64_t* value
);
