#pragma once

#include <memory>
#include <string>

#include "synthia/Action.hpp"
#include "synthia/Observation.hpp"
#include "synthia/World.hpp"

namespace synthia {

class Agent {
public:
    explicit Agent(std::string id, std::string goal = "navigate")
        : id_(std::move(id)), goal_(std::move(goal)) {}

    virtual ~Agent() = default;

    virtual Observation observe(const World& world) = 0;
    virtual Action decide(const Observation& observation) = 0;
    virtual void on_step_result(const Action& action, const SimulationStep& step, const World& world) {
        (void)action;
        (void)step;
        (void)world;
    }

    const std::string& id() const { return id_; }
    const std::string& goal() const { return goal_; }
    const std::vector<std::string>& inventory() const { return inventory_; }
    void add_item(const std::string& item) { inventory_.push_back(item); }
    bool has_item(const std::string& item) const {
        for (const auto& current : inventory_) {
            if (current == item) {
                return true;
            }
        }
        return false;
    }

protected:
    std::string id_;
    std::string goal_;
    std::vector<std::string> inventory_;
};

}  // namespace synthia
