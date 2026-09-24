#pragma once

#include <string>
#include <vector>

#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct Entity {
    EntityId id;
    std::string canonical_name;
    std::string entity_type = "unknown";
    std::vector<std::string> aliases;
    std::string description;

    bool operator==(const Entity& other) const {
        return id == other.id &&
               canonical_name == other.canonical_name &&
               entity_type == other.entity_type &&
               aliases == other.aliases &&
               description == other.description;
    }
};

}  // namespace knowledge
}  // namespace synthia
