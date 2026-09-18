#include "synthia/ScenarioRunner.hpp"

#include <cassert>
#include <cmath>

namespace {

class FixedAgent : public synthia::Agent {
public:
    FixedAgent() : synthia::Agent("fixed_agent", "navigate") {}

    synthia::Observation observe(const synthia::World& world) override {
        synthia::Observation observation;
        const auto& agent = world.entities().front();
        observation.agent_id = id();
        observation.goal = goal();
        observation.agent_position = agent.position;
        observation.target_position = world.target_position();
        const auto dx = observation.agent_position.x - observation.target_position.x;
        const auto dy = observation.agent_position.y - observation.target_position.y;
        const auto dz = observation.agent_position.z - observation.target_position.z;
        observation.distance_to_target = std::sqrt(dx * dx + dy * dy + dz * dz);
        observation.heading = agent.heading;
        observation.available_actions = {
            synthia::ActionType::MoveForward,
            synthia::ActionType::TurnLeft,
            synthia::ActionType::TurnRight,
            synthia::ActionType::Wait
        };
        return observation;
    }

    synthia::Action decide(const synthia::Observation& observation) override {
        if (observation.distance_to_target > 0.5) {
            return {synthia::ActionType::MoveForward, "", 5.0};
        }
        return {synthia::ActionType::Wait, "", 0.0};
    }
};

}  // namespace

int main() {
    synthia::WorldConfig config{"nav_demo", 42, 50, 50, 10, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {0.0, 0.0, 10.0};

    synthia::Scenario scenario;
    scenario.id = "nav_demo";
    scenario.name = "navigation";
    scenario.seed = 42;
    scenario.task.id = "move_to_goal";
    scenario.task.description = "move to goal";
    scenario.task.max_steps = 10;
    scenario.task.success_condition = "goal_reached";
    scenario.task.failure_condition = "failed";

    synthia::World world(config);
    FixedAgent agent;
    synthia::ScenarioRunner runner(world, agent, scenario.task, scenario);

    const auto result = runner.run();
    assert(result.termination == synthia::EpisodeTermination::Success);
    assert(result.steps >= 1);
    assert(runner.record().steps().size() == result.steps);
    assert(runner.record().steps().back().success);

    return 0;
}
