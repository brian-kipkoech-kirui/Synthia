#include "synthia/Agent.hpp"
#include "synthia/ScenarioRunner.hpp"
#include "synthia/World.hpp"

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

namespace {

class ResourceAgent : public synthia::Agent {
public:
    ResourceAgent(std::string id, synthia::Vec3 target)
        : synthia::Agent(std::move(id), "collect"), target_(std::move(target)) {}

    synthia::Observation observe(const synthia::World& world) override {
        synthia::Observation observation;
        observation.agent_id = id_;
        observation.goal = goal_;
        observation.status = "running";
        observation.agent_position = world.agent_position(id_);
        observation.target_position = target_;
        const auto dx = observation.agent_position.x - observation.target_position.x;
        const auto dy = observation.agent_position.y - observation.target_position.y;
        const auto dz = observation.agent_position.z - observation.target_position.z;
        observation.distance_to_target = std::sqrt(dx * dx + dy * dy + dz * dz);
        observation.available_actions = {
            synthia::ActionType::MoveLeft,
            synthia::ActionType::MoveRight,
            synthia::ActionType::MoveUp,
            synthia::ActionType::MoveDown,
            synthia::ActionType::Pickup,
            synthia::ActionType::Wait
        };
        for (const auto& entity : world.entities()) {
            if (entity.type == "resource" && entity.active) {
                observation.nearby_entities.push_back(entity);
                observation.nearby_resource_ids.push_back(entity.id);
            }
        }
        return observation;
    }

    synthia::Action decide(const synthia::Observation& observation) override {
        if (!observation.nearby_resource_ids.empty()) {
            return {synthia::ActionType::Pickup, observation.nearby_resource_ids.front(), 1.0};
        }
        if (observation.agent_position.x < observation.target_position.x) {
            return {synthia::ActionType::MoveRight, "", 1.0};
        }
        if (observation.agent_position.z < observation.target_position.z) {
            return {synthia::ActionType::MoveUp, "", 1.0};
        }
        return {synthia::ActionType::Wait, "", 0.0};
    }

private:
    synthia::Vec3 target_;
};

}  // namespace

int main() {
    synthia::Entity entity{"resource_0", "resource", {2.0, 0.0, 2.0}, {0.0, 0.0, 0.0}, 0.0, true, "ore"};
    assert(entity.id == "resource_0");
    assert(entity.type == "resource");
    assert(entity.active);

    synthia::WorldConfig config{"resource_collection", 42, 10, 10, 12, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {3.0, 0.0, 3.0};
    synthia::World world(config);
    world.add_resource("resource_0", {2.0, 0.0, 2.0}, "ore");
    world.add_agent("agent_0", {0.0, 0.0, 0.0}, {3.0, 0.0, 3.0});
    assert(world.has_agent("agent_0"));
    assert(world.is_valid_position({2.0, 0.0, 2.0}));

    ResourceAgent agent("agent_0", {3.0, 0.0, 3.0});
    const auto obs = agent.observe(world);
    assert(obs.agent_id == "agent_0");
    assert(!obs.nearby_resource_ids.empty());

    const auto move_right = world.step("agent_0", {synthia::ActionType::MoveRight, "", 1.0});
    const auto move_up = world.step("agent_0", {synthia::ActionType::MoveUp, "", 1.0});
    assert(move_right.message == "step executed");
    assert(move_up.message == "step executed");

    synthia::Action pickup{ synthia::ActionType::Pickup, "resource_0", 1.0 };
    const auto sim = world.step("agent_0", pickup);
    assert(sim.message == "resource collected");
    assert(world.simulation_time() >= 1);

    const auto snapshot = world.snapshot();
    assert(snapshot.simulation_time >= 0);
    assert(snapshot.entities.size() >= 1);

    synthia::Scenario scenario;
    scenario.id = "resource_collection";
    scenario.seed = 42;
    scenario.task.id = "collect_resources";
    scenario.task.description = "collect all required resources";
    scenario.task.max_steps = 6;
    scenario.task.success_condition = "all_resources_collected";

    synthia::ScenarioRunner runner(world, agent, scenario.task, scenario);
    const auto result = runner.run();
    assert(result.termination == synthia::EpisodeTermination::Success || result.termination == synthia::EpisodeTermination::Timeout || result.termination == synthia::EpisodeTermination::Failure);

    const auto replay = synthia::replay(runner.record());
    assert(replay.steps.size() == runner.record().steps().size());

    return 0;
}
