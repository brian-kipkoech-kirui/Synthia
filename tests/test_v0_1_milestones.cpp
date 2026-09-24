#include "synthia/ScenarioRunner.hpp"

#include <cassert>
#include <cmath>

namespace {

class MoveToTargetAgent : public synthia::Agent {
public:
    MoveToTargetAgent(std::string id, synthia::Vec3 target)
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
        observation.available_actions = {
            synthia::ActionType::MoveRight,
            synthia::ActionType::MoveUp,
            synthia::ActionType::Wait
        };
        return observation;
    }

    synthia::Action decide(const synthia::Observation& observation) override {
        if (observation.distance_to_target > 0.5) {
            if (observation.agent_position.x < observation.target_position.x) {
                return {synthia::ActionType::MoveRight, "", 1.0};
            }
            if (observation.agent_position.z < observation.target_position.z) {
                return {synthia::ActionType::MoveUp, "", 1.0};
            }
        }
        return {synthia::ActionType::Wait, "", 0.0};
    }

private:
    synthia::Vec3 target_;
};

}  // namespace

int main() {
    synthia::Task timeout_task;
    timeout_task.id = "ttl";
    timeout_task.max_steps = 3;
    assert(timeout_task.evaluate(0, false, false) == synthia::TaskStatus::Running);
    assert(timeout_task.evaluate(2, false, false) == synthia::TaskStatus::Timeout);

    synthia::WorldConfig config{"milestone_seed", 9, 12, 12, 5, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {3.0, 0.0, 3.0};

    synthia::World world_a(config);
    world_a.add_agent("agent_0", {0.0, 0.0, 0.0}, {3.0, 0.0, 3.0});
    synthia::World world_b(config);
    world_b.add_agent("agent_0", {0.0, 0.0, 0.0}, {3.0, 0.0, 3.0});

    synthia::Scenario scenario_a;
    scenario_a.id = "multi_step_nav";
    scenario_a.name = "multi_step_navigation";
    scenario_a.seed = 9;
    scenario_a.steps = {"advance", "advance", "settle"};
    scenario_a.task.id = "move_to_goal";
    scenario_a.task.description = "move to the target";
    scenario_a.task.max_steps = 3;
    scenario_a.task.success_condition = "goal_reached";
    scenario_a.task.failure_condition = "task_failed";

    synthia::Scenario scenario_b = scenario_a;
    MoveToTargetAgent agent_a("agent_0", {3.0, 0.0, 3.0});
    MoveToTargetAgent agent_b("agent_0", {3.0, 0.0, 3.0});

    synthia::ScenarioRunner runner_a(world_a, agent_a, scenario_a.task, scenario_a);
    synthia::ScenarioRunner runner_b(world_b, agent_b, scenario_b.task, scenario_b);

    const auto result_a = runner_a.run();
    const auto result_b = runner_b.run();
    assert(result_a.steps == scenario_a.steps.size());
    assert(result_a.steps == result_b.steps);
    assert(result_a.termination == result_b.termination);
    assert(result_a.total_reward == result_b.total_reward);
    assert(runner_a.record().steps().size() == scenario_a.steps.size());
    assert(runner_a.record().steps().size() == runner_b.record().steps().size());

    const auto replay_a = synthia::replay(runner_a.record());
    assert(replay_a.steps.size() == runner_a.record().steps().size());
    assert(replay_a.total_reward == result_a.total_reward);
    assert(replay_a.termination == result_a.termination);

    return 0;
}
