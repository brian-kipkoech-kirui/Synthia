#pragma once

#include <string>

#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct Relationship {
    RelationshipId id;
    EntityId source_entity_id;
    EntityId target_entity_id;
    std::string relationship_type;
    Timestamp valid_from = 0;
    Timestamp valid_to = kNoUpperBound;
    std::string description;
    ObservationId support_observation_id;

    bool operator==(const Relationship& other) const {
        return id == other.id &&
               source_entity_id == other.source_entity_id &&
               target_entity_id == other.target_entity_id &&
               relationship_type == other.relationship_type &&
               valid_from == other.valid_from &&
               valid_to == other.valid_to &&
               description == other.description &&
               support_observation_id == other.support_observation_id;
    }
};

}  // namespace knowledge
}  // namespace synthia
