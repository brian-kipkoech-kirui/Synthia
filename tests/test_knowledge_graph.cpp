#include "synthia/knowledge/KnowledgeGraph.hpp"

#include <cassert>

int main() {
    synthia::knowledge::KnowledgeGraph graph;

    synthia::knowledge::Entity alpha;
    alpha.id = "entity_alpha";
    alpha.canonical_name = "Alpha";
    alpha.entity_type = "company";

    synthia::knowledge::Entity beta;
    beta.id = "entity_beta";
    beta.canonical_name = "Beta";
    beta.entity_type = "company";

    synthia::knowledge::Observation obs;
    obs.id = "obs_001";
    obs.source_id = "src_001";
    obs.related_entity_ids = {"entity_alpha", "entity_beta"};

    graph.add_entity(alpha);
    graph.add_entity(beta);
    graph.add_observation(obs);

    assert(graph.contains_entity("entity_alpha"));
    assert(graph.get_entities_by_type("company").size() == 2);
    assert(graph.get_observations_for_entity("entity_alpha").size() == 1);
    return 0;
}
