#include "synthia/Action.hpp"
#include "synthia/Agent.hpp"
#include "synthia/ExperimentRecord.hpp"
#include "synthia/World.hpp"
#include "synthia/WorldConfig.hpp"

#include <cmath>
#include <iostream>
#include <memory>

namespace synthia {

class GoalSeekingAgent : public Agent {
public:
    GoalSeekingAgent(std::string id, std::string goal)
        : Agent(std::move(id), std::move(goal)) {}

    Observation observe(const World& world) override {
        Observation obs;
        obs.agent_id = id_;
        obs.goal = goal_;
        obs.status = "running";
        obs.target_position = world.target_position();
        obs.available_actions = {
            ActionType::MoveForward,
            ActionType::MoveBackward,
            ActionType::TurnLeft,
            ActionType::TurnRight,
            ActionType::Wait
        };

        if (!world.entities().empty()) {
            const auto& agent = world.entities().front();
            obs.agent_position = agent.position;
            obs.heading = agent.heading;

            const double dx = obs.agent_position.x - obs.target_position.x;
            const double dy = obs.agent_position.y - obs.target_position.y;
            const double dz = obs.agent_position.z - obs.target_position.z;
            obs.distance_to_target = std::sqrt(dx * dx + dy * dy + dz * dz);
        }
        return obs;
    }

    Action decide(const Observation& observation) override {
        if (observation.distance_to_target > 0.5) {
            return {ActionType::MoveForward, "", 5.0};
        }
        return {ActionType::Wait, "", 0.0};
    }
};

}  // namespace synthia

int main() {
    synthia::WorldConfig config{"demo_world", 42, 50, 50, 20, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {0.0, 0.0, 30.0};
    synthia::World world(config);

    synthia::GoalSeekingAgent agent("agent_0", "move_to_goal");
    synthia::ExperimentRecord record("nav_demo");

    for (int i = 0; i < config.max_steps; ++i) {
        const auto observation = agent.observe(world);
        const auto action = agent.decide(observation);
        const auto result = world.step(agent.id(), action);

        synthia::StepRecord step_record;
        step_record.step = i;
        step_record.observation = observation;
        step_record.action = action;
        step_record.success = result.success;
        step_record.reward = result.reward;
        step_record.message = result.message;
        record.add_step(step_record);

        std::cout << "step=" << i << " reward=" << result.reward << " done=" << result.done << "\n";

        if (result.done) {
            break;
        }
    }

    std::cout << "world_id=" << world.world_id() << "\n";
    std::cout << record.summary() << "\n";
    return 0;
}
