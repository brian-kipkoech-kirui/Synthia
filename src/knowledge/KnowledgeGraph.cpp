#include "synthia/knowledge/KnowledgeGraph.hpp"

#include <algorithm>

namespace synthia {
namespace knowledge {

void KnowledgeGraph::add_entity(const Entity& entity) {
    entities_[entity.id] = entity;
}

void KnowledgeGraph::add_event(const Event& event) {
    events_[event.id] = event;
    for (const auto& entity_id : event.entity_ids) {
        entity_events_[entity_id].insert(event.id);
    }
}

void KnowledgeGraph::add_relationship(const Relationship& relationship) {
    relationships_[relationship.id] = relationship;
    outgoing_edges_[relationship.source_entity_id].insert(relationship.target_entity_id);
}

void KnowledgeGraph::add_observation(const Observation& observation) {
    observations_[observation.id] = observation;
    for (const auto& entity_id : observation.related_entity_ids) {
        entity_observations_[entity_id].insert(observation.id);
    }
}

void KnowledgeGraph::add_source(const Source& source) {
    sources_[source.id] = source;
}

const std::map<EntityId, Entity>& KnowledgeGraph::entities() const { return entities_; }
const std::map<EventId, Event>& KnowledgeGraph::events() const { return events_; }
const std::map<RelationshipId, Relationship>& KnowledgeGraph::relationships() const { return relationships_; }
const std::map<ObservationId, Observation>& KnowledgeGraph::observations() const { return observations_; }
const std::map<SourceId, Source>& KnowledgeGraph::sources() const { return sources_; }

const Entity* KnowledgeGraph::get_entity(const EntityId& id) const {
    const auto it = entities_.find(id);
    return it == entities_.end() ? nullptr : &it->second;
}

std::vector<Entity> KnowledgeGraph::get_entities_by_type(const std::string& type) const {
    std::vector<Entity> values;
    for (const auto& [id, entity] : entities_) {
        if (entity.entity_type == type) {
            values.push_back(entity);
        }
    }
    return values;
}

std::vector<Event> KnowledgeGraph::get_events_for_entity(const EntityId& entity_id) const {
    std::vector<Event> values;
    const auto it = entity_events_.find(entity_id);
    if (it == entity_events_.end()) {
        return values;
    }
    for (const auto& event_id : it->second) {
        const auto event_it = events_.find(event_id);
        if (event_it != events_.end()) {
            values.push_back(event_it->second);
        }
    }
    return values;
}

std::vector<Entity> KnowledgeGraph::get_related_entities(const EntityId& entity_id) const {
    std::vector<Entity> values;
    const auto it = outgoing_edges_.find(entity_id);
    if (it == outgoing_edges_.end()) {
        return values;
    }
    for (const auto& adjacent_id : it->second) {
        const auto entity_it = entities_.find(adjacent_id);
        if (entity_it != entities_.end()) {
            values.push_back(entity_it->second);
        }
    }
    return values;
}

std::vector<Observation> KnowledgeGraph::get_observations_for_entity(const EntityId& entity_id) const {
    std::vector<Observation> values;
    const auto it = entity_observations_.find(entity_id);
    if (it == entity_observations_.end()) {
        return values;
    }
    for (const auto& observation_id : it->second) {
        const auto observation_it = observations_.find(observation_id);
        if (observation_it != observations_.end()) {
            values.push_back(observation_it->second);
        }
    }
    return values;
}

std::vector<Source> KnowledgeGraph::get_sources_for_entity(const EntityId& entity_id) const {
    std::vector<Source> values;
    for (const auto& observation : get_observations_for_entity(entity_id)) {
        const auto source_it = sources_.find(observation.source_id);
        if (source_it != sources_.end()) {
            values.push_back(source_it->second);
        }
    }
    return values;
}

bool KnowledgeGraph::contains_entity(const EntityId& id) const {
    return entities_.find(id) != entities_.end();
}

bool KnowledgeGraph::contains_event(const EventId& id) const {
    return events_.find(id) != events_.end();
}

}  // namespace knowledge
}  // namespace synthia
