#include "synthia/knowledge/KnowledgeIndex.hpp"

#include <cassert>

int main() {
    synthia::knowledge::KnowledgeIndex index;

    synthia::knowledge::Entity alpha;
    alpha.id = "entity_alpha";
    alpha.entity_type = "company";

    synthia::knowledge::Event event;
    event.id = "evt_001";
    event.timestamp = 150;
    event.event_type = "announcement";

    synthia::knowledge::Source source;
    source.id = "src_001";
    source.source_type = "news";

    synthia::knowledge::Observation observation;
    observation.id = "obs_001";
    observation.timestamp = 150;
    observation.source_id = source.id;
    observation.related_entity_ids = {"entity_alpha"};

    index.add_entity(alpha);
    index.add_event(event);
    index.add_source(source);
    index.add_observation(observation);

    assert(index.get_entity_by_id("entity_alpha") != nullptr);
    assert(index.get_event_by_id("evt_001") != nullptr);
    assert(index.get_observations_by_source("src_001").size() == 1);
    assert(index.get_events_for_time_range(100, 200).size() == 1);
    return 0;
}
