#include "synthia/knowledge/KnowledgeIndex.hpp"
#include "synthia/knowledge/WorldState.hpp"

#include <cassert>
#include <string>

int main() {
    synthia::knowledge::WorldState world;

    synthia::knowledge::Source news_source;
    news_source.id = "src_news";
    news_source.name = "Daily Ledger";
    news_source.source_type = "news";
    world.add_source(news_source);

    synthia::knowledge::Entity alpha;
    alpha.id = "alpha";
    alpha.canonical_name = "Alpha";
    alpha.entity_type = "company";
    world.add_entity(alpha);

    synthia::knowledge::Entity beta;
    beta.id = "beta";
    beta.canonical_name = "Beta";
    beta.entity_type = "company";
    world.add_entity(beta);

    synthia::knowledge::Observation observation;
    observation.id = "obs_1";
    observation.timestamp = 1000;
    observation.source_id = "src_news";
    observation.content = "Alpha and Beta signed a logistics partnership";
    observation.data_type = "news";
    observation.confidence = 0.94;
    observation.related_entity_ids = {"alpha", "beta"};
    world.apply(observation);

    synthia::knowledge::Event event;
    event.id = "evt_1";
    event.timestamp = 1000;
    event.event_type = "partnership";
    event.description = "Alpha and Beta coordinate logistics";
    event.entity_ids = {"alpha", "beta"};
    world.add_event(event);

    synthia::knowledge::Relationship relationship;
    relationship.id = "rel_1";
    relationship.source_entity_id = "alpha";
    relationship.target_entity_id = "beta";
    relationship.relationship_type = "partners_with";
    relationship.valid_from = 1000;
    relationship.valid_to = 2000;
    relationship.description = "Alpha partners with Beta";
    relationship.support_observation_id = "obs_1";
    world.add_relationship(relationship);

    auto alpha_events = world.get_events_for_entity("alpha");
    auto related = world.get_relationships("alpha");
    auto obs_for_alpha = world.get_observations_for_entity("alpha");

    assert(world.contains_entity("alpha"));
    assert(world.contains_entity("beta"));
    assert(alpha_events.size() == 1);
    assert(related.size() == 1);
    assert(obs_for_alpha.size() == 1);
    assert(world.index().get_entity_by_id("alpha") != nullptr);
    assert(world.index().get_event_by_id("evt_1") != nullptr);
    assert(world.index().get_relationships_for_entity("alpha").size() == 1);
    assert(world.index().get_events_for_time_range(900, 1100).size() == 1);

    return 0;
}
