#include "synthia/knowledge/WorldState.hpp"

#include <cassert>

int main() {
    synthia::knowledge::WorldState world;

    synthia::knowledge::Entity alpha;
    alpha.id = "entity_alpha";
    alpha.canonical_name = "Alpha";
    alpha.entity_type = "company";

    synthia::knowledge::Source source;
    source.id = "src_001";
    source.name = "newswire";
    source.source_type = "news";

    synthia::knowledge::Observation obs;
    obs.id = "obs_001";
    obs.timestamp = 100;
    obs.source_id = source.id;
    obs.content = "Alpha expands to Nairobi";
    obs.related_entity_ids = {"entity_alpha"};

    world.add_source(source);
    world.add_entity(alpha);
    world.apply(obs);

    assert(world.contains_entity("entity_alpha"));
    assert(world.get_observations_for_entity("entity_alpha").size() == 1);
    assert(world.sources().count("src_001") == 1u);
    return 0;
}
