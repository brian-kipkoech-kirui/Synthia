#pragma once

#include <string>

#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct Provenance {
    ObservationId observation_id;
    SourceId source_id;
    Timestamp observed_at = 0;
    double confidence = 1.0;
    std::string notes;

    bool operator==(const Provenance& other) const {
        return observation_id == other.observation_id &&
               source_id == other.source_id &&
               observed_at == other.observed_at &&
               confidence == other.confidence &&
               notes == other.notes;
    }
};

}  // namespace knowledge
}  // namespace synthia
