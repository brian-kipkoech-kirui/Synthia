#pragma once

#include <cstdint>
#include <string>

#include "synthia/Entity.hpp"

namespace synthia {

struct WorldConfig {
    std::string name = "default_world";
    std::uint32_t seed = 1;
    int width = 100;
    int height = 100;
    int max_steps = 100;
    std::string climate = "urban";

    Vec3 start_position{0.0, 0.0, 0.0};
    Vec3 target_position{15.0, 0.0, 30.0};
    std::string scenario = "navigation";
};

}  // namespace synthia
