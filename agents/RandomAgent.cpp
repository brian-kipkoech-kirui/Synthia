#include "synthia/Agent.hpp"

#include <cstdlib>

namespace synthia {

class TestNavigationAgent : public Agent {
public:
    explicit TestNavigationAgent(std::string id, std::string goal = "navigate_to_goal")
        : Agent(std::move(id), std::move(goal)) {}

    Observation observe(const World& world) override {
        Observation obs;
        obs.agent_id = id_;
        obs.goal = goal_;
        obs.status = "running";
        if (!world.entities().empty()) {
            obs.position = world.entities().front().position;
            obs.nearby_entities = world.entities();
        }
        return obs;
    }

    Action decide(const Observation& observation) override {
        if (observation.position.z < 30.0) {
            return {ActionType::MoveForward, "", 5.0};
        }
        return {ActionType::Wait, "", 0.0};
    }
};

}  // namespace synthia
