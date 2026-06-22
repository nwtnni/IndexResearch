#include <memory>
#include <optional>

#include "wrap.h"

std::unique_ptr<ParlayUnorderedMap> unorderedmap_new() { return std::make_unique<ParlayUnorderedMap>(); }

void map_upsert(ParlayUnorderedMap *map, uint64_t key, uint64_t value) {
    map->inner.Upsert(
        key,
        [value](std::optional<uint64_t>) {
            return value;
        }
    );
}

void map_update(ParlayUnorderedMap *map, uint64_t key, uint64_t value) {
    map->inner.Insert(key, value);
}

uint64_t map_find(ParlayUnorderedMap *map, uint64_t key) {
    auto res = map->inner.Find(key);
    if (res == std::nullopt) {
        throw std::out_of_range("key not in map");
    }
    return res.value();
}
