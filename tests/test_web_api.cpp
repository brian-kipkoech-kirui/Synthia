#include "synthia/api/DashboardApi.hpp"

#include <cassert>
#include <string>

int main() {
    auto world = synthia::api::create_demo_world();
    const auto status = synthia::api::make_status(world);
    assert(status.entity_count >= 2);
    assert(status.event_count >= 2);
    assert(status.observation_count >= 2);
    assert(status.relationship_count >= 2);

    const auto entities = synthia::api::list_entities(world);
    assert(!entities.empty());

    const auto graph = synthia::api::make_graph(world);
    assert(!graph.nodes.empty());
    assert(!graph.edges.empty());

    const auto state = synthia::api::make_world_state(world, 20250301);
    assert(!state.entities.empty());

    const auto search = synthia::api::search(world, "Company Alpha");
    assert(!search.empty());

    const auto provenance = synthia::api::make_provenance(world, "rel_alpha_beta");
    assert(!provenance.chain.empty());

    return 0;
}
