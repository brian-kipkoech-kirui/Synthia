#pragma once

#include <string>
#include <vector>

#include "synthia/Action.hpp"
#include "synthia/Entity.hpp"
#include "synthia/World.hpp"

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
    double energy = 0.0;
    int simulation_time = 0;
    TerrainType terrain = TerrainType::Plain;
    EnvironmentState environment;
    std::vector<std::string> nearby_obstacle_ids;
    std::vector<WorldEvent> recent_events;
};

}  // namespace synthia
