#pragma once

#include <string>
#include <vector>

#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct Observation {
    ObservationId id;
    Timestamp timestamp = 0;
    SourceId source_id;
    std::string content;
    std::string data_type = "unknown";
    double confidence = 1.0;
    std::vector<EntityId> related_entity_ids;
    std::vector<EventId> related_event_ids;

    bool operator==(const Observation& other) const {
        return id == other.id &&
               timestamp == other.timestamp &&
               source_id == other.source_id &&
               content == other.content &&
               data_type == other.data_type &&
               confidence == other.confidence;
    }
};

}  // namespace knowledge
}  // namespace synthia
