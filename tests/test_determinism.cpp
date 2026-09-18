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

synthia::Scenario make_scenario(int seed) {
    synthia::Scenario scenario;
    scenario.id = "nav_demo";
    scenario.name = "navigation";
    scenario.seed = seed;
    scenario.task.id = "move_to_goal";
    scenario.task.description = "move to goal";
    scenario.task.max_steps = 10;
    scenario.task.success_condition = "goal_reached";
    scenario.task.failure_condition = "failed";
    return scenario;
}

}  // namespace

int main() {
    synthia::WorldConfig config_a{"nav_demo", 42, 50, 50, 10, "urban"};
    config_a.start_position = {0.0, 0.0, 0.0};
    config_a.target_position = {0.0, 0.0, 10.0};

    synthia::WorldConfig config_b{"nav_demo", 42, 50, 50, 10, "urban"};
    config_b.start_position = {0.0, 0.0, 0.0};
    config_b.target_position = {0.0, 0.0, 10.0};

    FixedAgent agent_a;
    FixedAgent agent_b;
    synthia::World world_a(config_a);
    synthia::World world_b(config_b);

    synthia::Scenario scenario_a = make_scenario(42);
    synthia::Scenario scenario_b = make_scenario(42);

    synthia::ScenarioRunner runner_a(world_a, agent_a, scenario_a.task, scenario_a);
    synthia::ScenarioRunner runner_b(world_b, agent_b, scenario_b.task, scenario_b);

    const auto result_a = runner_a.run();
    const auto result_b = runner_b.run();

    assert(result_a.termination == result_b.termination);
    assert(result_a.total_reward == result_b.total_reward);
    assert(result_a.steps == result_b.steps);
    assert(runner_a.record().steps().size() == runner_b.record().steps().size());

    const auto replay_a = synthia::replay(runner_a.record());
    const auto replay_b = synthia::replay(runner_b.record());
    assert(replay_a.steps.size() == replay_b.steps.size());
    for (size_t i = 0; i < replay_a.steps.size(); ++i) {
        assert(replay_a.steps[i] == replay_b.steps[i]);
    }

    synthia::World world_c(config_a);
    synthia::Scenario scenario_c = make_scenario(43);
    FixedAgent agent_c;
    synthia::ScenarioRunner runner_c(world_c, agent_c, scenario_c.task, scenario_c);
    const auto result_c = runner_c.run();
    assert(result_a.termination == result_c.termination || result_a.total_reward != result_c.total_reward || result_a.steps != result_c.steps);

    return 0;
}
