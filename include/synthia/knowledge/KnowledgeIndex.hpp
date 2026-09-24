#pragma once

#include <map>
#include <string>
#include <vector>

#include "synthia/knowledge/Entity.hpp"
#include "synthia/knowledge/Event.hpp"
#include "synthia/knowledge/Observation.hpp"
#include "synthia/knowledge/Relationship.hpp"
#include "synthia/knowledge/Source.hpp"

namespace synthia {
namespace knowledge {

class KnowledgeIndex {
public:
    void add_entity(const Entity& entity);
    void add_event(const Event& event);
    void add_observation(const Observation& observation);
    void add_relationship(const Relationship& relationship);
    void add_source(const Source& source);

    const Entity* get_entity_by_id(const EntityId& id) const;
    const Event* get_event_by_id(const EventId& id) const;
    const Observation* get_observation_by_id(const ObservationId& id) const;
    const Source* get_source_by_id(const SourceId& id) const;

    std::vector<Entity> get_entities_by_type(const std::string& type) const;
    std::vector<Event> get_events_by_type(const std::string& event_type) const;
    std::vector<Observation> get_observations_by_source(const SourceId& source_id) const;
    std::vector<Relationship> get_relationships_for_entity(const EntityId& id) const;
    std::vector<Observation> get_observations_for_entity(const EntityId& id) const;
    std::vector<Event> get_events_for_time_range(Timestamp start, Timestamp end) const;

    const std::map<EntityId, Entity>& entities() const;
    const std::map<EventId, Event>& events() const;
    const std::map<ObservationId, Observation>& observations() const;
    const std::map<RelationshipId, Relationship>& relationships() const;
    const std::map<SourceId, Source>& sources() const;

private:
    std::map<EntityId, Entity> entities_;
    std::map<EventId, Event> events_;
    std::map<ObservationId, Observation> observations_;
    std::map<RelationshipId, Relationship> relationships_;
    std::map<SourceId, Source> sources_;

    std::map<std::string, std::vector<EntityId>> entities_by_type_;
    std::map<std::string, std::vector<EventId>> events_by_type_;
    std::map<SourceId, std::vector<ObservationId>> observations_by_source_;
    std::map<EntityId, std::vector<RelationshipId>> relationships_for_entity_;
    std::map<EntityId, std::vector<ObservationId>> observations_for_entity_;
    std::map<Timestamp, std::vector<EventId>> events_by_time_;
};

}  // namespace knowledge
}  // namespace synthia
