#include "synthia/knowledge/Entity.hpp"

#include <cassert>

int main() {
    synthia::knowledge::Entity entity;
    entity.id = "entity_alpha";
    entity.canonical_name = "Alpha";
    entity.entity_type = "company";
    entity.aliases = {"Alpha Corp", "Alpha Group"};

    assert(entity.id == "entity_alpha");
    assert(entity.entity_type == "company");
    assert(entity.aliases.size() == 2);
    return 0;
}
