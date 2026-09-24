#pragma once

#include <string>
#include <vector>

#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct Event {
    EventId id;
    Timestamp timestamp = 0;
    std::string event_type = "unknown";
    std::string description;
    std::vector<EntityId> entity_ids;

    bool operator==(const Event& other) const {
        return id == other.id &&
               timestamp == other.timestamp &&
               event_type == other.event_type &&
               description == other.description &&
               entity_ids == other.entity_ids;
    }
};

}  // namespace knowledge
}  // namespace synthia
