#include "synthia/Action.hpp"
#include "synthia/Observation.hpp"
#include "synthia/World.hpp"
#include "synthia/WorldConfig.hpp"

#include <cassert>
#include <iostream>

int main() {
    synthia::WorldConfig config{"nav_demo", 42, 50, 50, 10, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {15.0, 0.0, 30.0};
    synthia::World world(config);

    assert(world.seed() == 42);
    assert(!world.world_id().empty());
    assert(world.entities().size() >= 1);
    assert(world.world_id() == "nav_demo_42");

    synthia::Observation observation;
    observation.agent_id = "agent_0";
    observation.agent_position = {0.0, 0.0, 0.0};
    observation.target_position = {15.0, 0.0, 30.0};
    observation.distance_to_target = 30.0;
    observation.heading = 0.0;
    observation.available_actions = {
        synthia::ActionType::MoveForward,
        synthia::ActionType::TurnLeft,
        synthia::ActionType::TurnRight,
        synthia::ActionType::Wait
    };
    assert(observation.available_actions.size() == 4);

    synthia::Action move{ synthia::ActionType::MoveForward, "", 5.0 };
    auto result = world.step("agent_0", move);
    assert(world.step_count() >= 1);
    assert(result.message == "step executed" || result.message == "agent reached goal");

    std::cout << "world core ok\n";
    return 0;
}
