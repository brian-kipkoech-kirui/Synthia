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

    const std::string& id() const { return id_; }
    const std::string& goal() const { return goal_; }

protected:
    std::string id_;
    std::string goal_;
};

}  // namespace synthia
