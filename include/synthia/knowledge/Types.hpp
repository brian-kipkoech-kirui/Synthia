#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace synthia {
namespace knowledge {

using Timestamp = std::int64_t;
using ObservationId = std::string;
using SourceId = std::string;
using EntityId = std::string;
using EventId = std::string;
using RelationshipId = std::string;

constexpr Timestamp kNoUpperBound = std::numeric_limits<Timestamp>::max();

}  // namespace knowledge
}  // namespace synthia
