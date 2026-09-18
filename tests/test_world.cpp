#include "synthia/world.hpp"

#include <cassert>

int main() {
    synthia::WorldConfig config_a{"demo", 1234, "tropical"};
    synthia::WorldConfig config_b{"demo", 1234, "tropical"};

    synthia::World world_a(config_a);
    synthia::World world_b(config_b);

    assert(world_a.seed() == world_b.seed());
    assert(world_a.world_id() == world_b.world_id());
    assert(world_a.time() == world_b.time());
    assert(world_a.entities().size() == world_b.entities().size());

    synthia::World world_c({"demo", 7, "urban"});
    assert(!world_c.entities().empty());

    return 0;
}
