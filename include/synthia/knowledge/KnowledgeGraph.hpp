#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "synthia/knowledge/Entity.hpp"
#include "synthia/knowledge/Event.hpp"
#include "synthia/knowledge/Observation.hpp"
#include "synthia/knowledge/Relationship.hpp"
#include "synthia/knowledge/Source.hpp"

namespace synthia {
namespace knowledge {

class KnowledgeGraph {
public:
    void add_entity(const Entity& entity);
    void add_event(const Event& event);
    void add_relationship(const Relationship& relationship);
    void add_observation(const Observation& observation);
    void add_source(const Source& source);

    const std::map<EntityId, Entity>& entities() const;
    const std::map<EventId, Event>& events() const;
    const std::map<RelationshipId, Relationship>& relationships() const;
    const std::map<ObservationId, Observation>& observations() const;
    const std::map<SourceId, Source>& sources() const;

    const Entity* get_entity(const EntityId& id) const;
    std::vector<Entity> get_entities_by_type(const std::string& type) const;
    std::vector<Event> get_events_for_entity(const EntityId& entity_id) const;
    std::vector<Entity> get_related_entities(const EntityId& entity_id) const;
    std::vector<Observation> get_observations_for_entity(const EntityId& entity_id) const;
    std::vector<Source> get_sources_for_entity(const EntityId& entity_id) const;

    bool contains_entity(const EntityId& id) const;
    bool contains_event(const EventId& id) const;

private:
    std::map<EntityId, Entity> entities_;
    std::map<EventId, Event> events_;
    std::map<RelationshipId, Relationship> relationships_;
    std::map<ObservationId, Observation> observations_;
    std::map<SourceId, Source> sources_;

    std::map<EntityId, std::set<EntityId>> outgoing_edges_;
    std::map<EntityId, std::set<EventId>> entity_events_;
    std::map<EntityId, std::set<ObservationId>> entity_observations_;
};

}  // namespace knowledge
}  // namespace synthia
