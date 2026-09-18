#pragma once

#include <string>
#include <vector>

#include "synthia/Action.hpp"
#include "synthia/Entity.hpp"

namespace synthia {

struct Observation {
    std::string agent_id;
    std::string goal;
    std::string status = "idle";

    Vec3 agent_position;
    Vec3 target_position;
    double distance_to_target = 0.0;
    double heading = 0.0;
    std::vector<ActionType> available_actions;

    std::vector<Entity> nearby_entities;
    std::vector<std::string> nearby_resource_ids;
    std::vector<std::string> visible_entities;
    double reward = 0.0;
};

}  // namespace synthia
