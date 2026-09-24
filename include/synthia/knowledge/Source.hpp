#pragma once

#include <string>

#include "synthia/knowledge/Types.hpp"

namespace synthia {
namespace knowledge {

struct Source {
    SourceId id;
    std::string name;
    std::string uri;
    std::string source_type = "unknown";

    bool operator==(const Source& other) const {
        return id == other.id && name == other.name && uri == other.uri && source_type == other.source_type;
    }
};

}  // namespace knowledge
}  // namespace synthia
