#pragma once

#include <string>
#include <vector>

#include "synthia/knowledge/Observation.hpp"
#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct Hypothesis {
    std::string id;
    std::string description;
    std::vector<ObservationId> supporting_observations;
    std::vector<std::string> assumptions;
    double confidence = 0.0;
    Timestamp created_at = 0;
};

}  // namespace knowledge
}  // namespace synthia
