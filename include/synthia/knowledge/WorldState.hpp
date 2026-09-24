#pragma once

#include <map>
#include <string>
#include <vector>

#include "synthia/knowledge/Entity.hpp"
#include "synthia/knowledge/Event.hpp"
#include "synthia/knowledge/KnowledgeGraph.hpp"
#include "synthia/knowledge/KnowledgeIndex.hpp"
#include "synthia/knowledge/Observation.hpp"
#include "synthia/knowledge/Relationship.hpp"
#include "synthia/knowledge/Source.hpp"
#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct TemporalFact {
    std::string subject_id;
    std::string field;
    std::string value;
    Timestamp valid_from = 0;
    Timestamp valid_to = kNoUpperBound;
};

class WorldState {
public:
    WorldState();

    void apply(const Observation& observation);
    void add_entity(const Entity& entity);
    void add_event(const Event& event);
    void add_relationship(const Relationship& relationship);
    void add_source(const Source& source);

    const std::map<EntityId, Entity>& entities() const;
    const std::map<EventId, Event>& events() const;
    const std::map<RelationshipId, Relationship>& relationships() const;
    const std::map<ObservationId, Observation>& observations() const;
    const std::map<SourceId, Source>& sources() const;
    const std::vector<TemporalFact>& history() const;

    const Entity* get_entity(const EntityId& id) const;
    std::vector<Event> get_events_for_entity(const EntityId& entity_id) const;
    std::vector<Relationship> get_relationships(const EntityId& entity_id) const;
    std::vector<Observation> get_observations_for_entity(const EntityId& entity_id) const;
    std::vector<Source> get_sources_for_entity(const EntityId& entity_id) const;

    bool contains_entity(const EntityId& id) const;
    bool contains_event(const EventId& id) const;

    const KnowledgeGraph& graph() const;
    const KnowledgeIndex& index() const;

private:
    KnowledgeGraph graph_;
    KnowledgeIndex index_;
    std::vector<TemporalFact> history_;
};

}  // namespace knowledge
}  // namespace synthia
