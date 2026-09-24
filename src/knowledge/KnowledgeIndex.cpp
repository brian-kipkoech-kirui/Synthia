#include "synthia/knowledge/KnowledgeIndex.hpp"

#include <algorithm>

namespace synthia {
namespace knowledge {

void KnowledgeIndex::add_entity(const Entity& entity) {
    entities_[entity.id] = entity;
    entities_by_type_[entity.entity_type].push_back(entity.id);
}

void KnowledgeIndex::add_event(const Event& event) {
    events_[event.id] = event;
    events_by_type_[event.event_type].push_back(event.id);
    events_by_time_[event.timestamp].push_back(event.id);
}

void KnowledgeIndex::add_observation(const Observation& observation) {
    observations_[observation.id] = observation;
    observations_by_source_[observation.source_id].push_back(observation.id);
    for (const auto& entity_id : observation.related_entity_ids) {
        observations_for_entity_[entity_id].push_back(observation.id);
    }
}

void KnowledgeIndex::add_relationship(const Relationship& relationship) {
    relationships_[relationship.id] = relationship;
    relationships_for_entity_[relationship.source_entity_id].push_back(relationship.id);
    relationships_for_entity_[relationship.target_entity_id].push_back(relationship.id);
}

void KnowledgeIndex::add_source(const Source& source) {
    sources_[source.id] = source;
}

const Entity* KnowledgeIndex::get_entity_by_id(const EntityId& id) const {
    const auto it = entities_.find(id);
    return it == entities_.end() ? nullptr : &it->second;
}

const Event* KnowledgeIndex::get_event_by_id(const EventId& id) const {
    const auto it = events_.find(id);
    return it == events_.end() ? nullptr : &it->second;
}

const Observation* KnowledgeIndex::get_observation_by_id(const ObservationId& id) const {
    const auto it = observations_.find(id);
    return it == observations_.end() ? nullptr : &it->second;
}

const Source* KnowledgeIndex::get_source_by_id(const SourceId& id) const {
    const auto it = sources_.find(id);
    return it == sources_.end() ? nullptr : &it->second;
}

std::vector<Entity> KnowledgeIndex::get_entities_by_type(const std::string& type) const {
    std::vector<Entity> values;
    const auto it = entities_by_type_.find(type);
    if (it == entities_by_type_.end()) {
        return values;
    }
    for (const auto& entity_id : it->second) {
        const auto entity_it = entities_.find(entity_id);
        if (entity_it != entities_.end()) {
            values.push_back(entity_it->second);
        }
    }
    return values;
}

std::vector<Event> KnowledgeIndex::get_events_by_type(const std::string& event_type) const {
    std::vector<Event> values;
    const auto it = events_by_type_.find(event_type);
    if (it == events_by_type_.end()) {
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

std::vector<Observation> KnowledgeIndex::get_observations_by_source(const SourceId& source_id) const {
    std::vector<Observation> values;
    const auto it = observations_by_source_.find(source_id);
    if (it == observations_by_source_.end()) {
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

std::vector<Relationship> KnowledgeIndex::get_relationships_for_entity(const EntityId& id) const {
    std::vector<Relationship> values;
    const auto it = relationships_for_entity_.find(id);
    if (it == relationships_for_entity_.end()) {
        return values;
    }
    for (const auto& relationship_id : it->second) {
        const auto relationship_it = relationships_.find(relationship_id);
        if (relationship_it != relationships_.end()) {
            values.push_back(relationship_it->second);
        }
    }
    return values;
}

std::vector<Observation> KnowledgeIndex::get_observations_for_entity(const EntityId& id) const {
    std::vector<Observation> values;
    const auto it = observations_for_entity_.find(id);
    if (it == observations_for_entity_.end()) {
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

std::vector<Event> KnowledgeIndex::get_events_for_time_range(Timestamp start, Timestamp end) const {
    std::vector<Event> values;
    for (const auto& [time, ids] : events_by_time_) {
        if (time >= start && time <= end) {
            for (const auto& event_id : ids) {
                const auto event_it = events_.find(event_id);
                if (event_it != events_.end()) {
                    values.push_back(event_it->second);
                }
            }
        }
    }
    return values;
}

const std::map<EntityId, Entity>& KnowledgeIndex::entities() const { return entities_; }
const std::map<EventId, Event>& KnowledgeIndex::events() const { return events_; }
const std::map<ObservationId, Observation>& KnowledgeIndex::observations() const { return observations_; }
const std::map<RelationshipId, Relationship>& KnowledgeIndex::relationships() const { return relationships_; }
const std::map<SourceId, Source>& KnowledgeIndex::sources() const { return sources_; }

}  // namespace knowledge
}  // namespace synthia
