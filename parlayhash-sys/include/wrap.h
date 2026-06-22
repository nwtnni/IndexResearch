#include <cstdint>
#include <memory>

#include "parlay_hash/unordered_map.h"

class ParlayUnorderedMap {
public:
  parlay::parlay_unordered_map<uint64_t, uint64_t> inner;
};

std::unique_ptr<ParlayUnorderedMap> unorderedmap_new();

void map_upsert(ParlayUnorderedMap *map, uint64_t key, uint64_t value);

void map_update(ParlayUnorderedMap *map, uint64_t key, uint64_t value);

uint64_t map_find(ParlayUnorderedMap *map, uint64_t key);
