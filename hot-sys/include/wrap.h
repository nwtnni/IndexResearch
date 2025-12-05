#include <cstdint>
#include <memory>

class HOTTree {
public:
    HOTTree();
    ~HOTTree();

    bool upsert(std::uint64_t key, std::uint32_t value);
    bool search(std::uint64_t key, std::uint32_t* value);

private:
    void* inner_; 
};

std::unique_ptr<HOTTree> hottree_new();

bool hottree_upsert(HOTTree *tree, uint64_t key, uint32_t val);

bool hottree_search(HOTTree *tree, uint64_t key, uint32_t *val);
