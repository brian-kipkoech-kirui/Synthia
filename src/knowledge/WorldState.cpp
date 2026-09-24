#include "synthia/knowledge/WorldState.hpp"

namespace synthia {
namespace knowledge {

WorldState::WorldState() = default;

void WorldState::apply(const Observation& observation) {
    graph_.add_observation(observation);
    index_.add_observation(observation);
    if (!observation.source_id.empty()) {
        // source registration occurs via add_source when data is ingested
    }
    for (const auto& entity_id : observation.related_entity_ids) {
        const auto* entity = graph_.get_entity(entity_id);
        if (entity == nullptr) {
            Entity fallback;
            fallback.id = entity_id;
            fallback.canonical_name = entity_id;
            fallback.entity_type = "unknown";
            add_entity(fallback);
        }
    }
}

void WorldState::add_entity(const Entity& entity) {
    graph_.add_entity(entity);
    index_.add_entity(entity);
    history_.push_back({entity.id, "entity_created", entity.canonical_name, 0, kNoUpperBound});
}

void WorldState::add_event(const Event& event) {
    graph_.add_event(event);
    index_.add_event(event);
    history_.push_back({event.id, "event_created", event.event_type, event.timestamp, kNoUpperBound});
}

void WorldState::add_relationship(const Relationship& relationship) {
    graph_.add_relationship(relationship);
    index_.add_relationship(relationship);
    history_.push_back({relationship.id, "relationship_created", relationship.relationship_type, relationship.valid_from, relationship.valid_to});
}

void WorldState::add_source(const Source& source) {
    graph_.add_source(source);
    index_.add_source(source);
}

const std::map<EntityId, Entity>& WorldState::entities() const { return graph_.entities(); }
const std::map<EventId, Event>& WorldState::events() const { return graph_.events(); }
const std::map<RelationshipId, Relationship>& WorldState::relationships() const { return graph_.relationships(); }
const std::map<ObservationId, Observation>& WorldState::observations() const { return graph_.observations(); }
const std::map<SourceId, Source>& WorldState::sources() const { return graph_.sources(); }
const std::vector<TemporalFact>& WorldState::history() const { return history_; }

const Entity* WorldState::get_entity(const EntityId& id) const { return graph_.get_entity(id); }

std::vector<Event> WorldState::get_events_for_entity(const EntityId& entity_id) const {
    return graph_.get_events_for_entity(entity_id);
}

std::vector<Relationship> WorldState::get_relationships(const EntityId& entity_id) const {
    std::vector<Relationship> result;
    for (const auto& [id, relationship] : graph_.relationships()) {
        if (relationship.source_entity_id == entity_id || relationship.target_entity_id == entity_id) {
            result.push_back(relationship);
        }
    }
    return result;
}

std::vector<Observation> WorldState::get_observations_for_entity(const EntityId& entity_id) const {
    return graph_.get_observations_for_entity(entity_id);
}

std::vector<Source> WorldState::get_sources_for_entity(const EntityId& entity_id) const {
    return graph_.get_sources_for_entity(entity_id);
}

bool WorldState::contains_entity(const EntityId& id) const { return graph_.contains_entity(id); }
bool WorldState::contains_event(const EventId& id) const { return graph_.contains_event(id); }

const KnowledgeGraph& WorldState::graph() const { return graph_; }
const KnowledgeIndex& WorldState::index() const { return index_; }

}  // namespace knowledge
}  // namespace synthia
