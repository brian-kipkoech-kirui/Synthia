#include "synthia/ScenarioRunner.hpp"

#include <cassert>
#include <cmath>
#include <vector>

namespace {

class NavigationAgent : public synthia::Agent {
public:
    NavigationAgent(std::string id, synthia::Vec3 target)
        : synthia::Agent(std::move(id), "navigate"), target_(std::move(target)) {}

    synthia::Observation observe(const synthia::World& world) override {
        synthia::Observation observation;
        observation.agent_id = id();
        observation.goal = goal();
        observation.agent_position = world.agent_position(id());
        observation.target_position = target_;
        const auto dx = observation.agent_position.x - observation.target_position.x;
        const auto dy = observation.agent_position.y - observation.target_position.y;
        const auto dz = observation.agent_position.z - observation.target_position.z;
        observation.distance_to_target = std::sqrt(dx * dx + dy * dy + dz * dz);
        observation.heading = 0.0;
        observation.available_actions = {
            synthia::ActionType::MoveLeft,
            synthia::ActionType::MoveRight,
            synthia::ActionType::MoveUp,
            synthia::ActionType::MoveDown,
            synthia::ActionType::Wait
        };
        return observation;
    }

    synthia::Action decide(const synthia::Observation& observation) override {
        if (observation.distance_to_target > 0.5) {
            if (observation.agent_position.x > observation.target_position.x) {
                return {synthia::ActionType::MoveLeft, "", 1.0};
            }
            if (observation.agent_position.x < observation.target_position.x) {
                return {synthia::ActionType::MoveRight, "", 1.0};
            }
            if (observation.agent_position.z > observation.target_position.z) {
                return {synthia::ActionType::MoveDown, "", 1.0};
            }
            return {synthia::ActionType::MoveUp, "", 1.0};
        }
        return {synthia::ActionType::Wait, "", 0.0};
    }

private:
    synthia::Vec3 target_;
};

}  // namespace

int main() {
    synthia::WorldConfig config{"multi_agent_nav", 42, 12, 12, 10, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {3.0, 0.0, 3.0};

    synthia::World world(config);
    world.add_agent("agent_0", {0.0, 0.0, 0.0}, {3.0, 0.0, 3.0});
    world.add_agent("agent_1", {0.0, 0.0, 1.0}, {2.0, 0.0, 2.0});

    NavigationAgent agent_0("agent_0", {3.0, 0.0, 3.0});
    NavigationAgent agent_1("agent_1", {2.0, 0.0, 2.0});
    std::vector<std::reference_wrapper<synthia::Agent>> agents = {agent_0, agent_1};

    synthia::Scenario scenario;
    scenario.id = "multi_agent_nav";
    scenario.name = "multi_agent_navigation";
    scenario.seed = 42;
    scenario.task.id = "multi_agent_task";
    scenario.task.description = "navigate to targets";
    scenario.task.max_steps = 6;
    scenario.task.success_condition = "all_agents_reached_targets";

    synthia::ScenarioRunner runner(world, agents, scenario.task, scenario);
    const auto result = runner.run();

    assert(result.steps >= 1);
    assert(runner.record().steps().size() == result.steps);
    assert(runner.record().steps().front().agent_steps.size() == 2);
    assert(runner.record().steps().front().agent_steps[0].agent_id == "agent_0");
    assert(runner.record().steps().front().agent_steps[1].agent_id == "agent_1");
    assert(result.termination == synthia::EpisodeTermination::Success || result.termination == synthia::EpisodeTermination::Timeout);

    const auto replay = synthia::replay(runner.record());
    assert(replay.steps.size() == result.steps);

    return 0;
}
