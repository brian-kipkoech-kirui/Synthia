#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "synthia/Action.hpp"
#include "synthia/Entity.hpp"
#include "synthia/WorldConfig.hpp"

namespace synthia {

struct SimulationStep {
    bool success = false;
    bool done = false;
    double reward = 0.0;
    std::string message;
};

class World {
public:
    explicit World(const WorldConfig& config);

    const std::string& world_id() const;
    std::uint32_t seed() const;
    int width() const;
    int height() const;
    int step_count() const;
    double time() const;
    const std::vector<Entity>& entities() const;
    const Vec3& start_position() const;
    const Vec3& target_position() const;

    void add_entity(const Entity& entity);
    void set_agent_position(const Vec3& position);
    void tick(double dt = 1.0);
    SimulationStep step(const std::string& agent_id, const Action& action);
    void reset();

private:
    std::string world_id_;
    std::uint32_t seed_;
    WorldConfig config_;
    Vec3 start_position_;
    Vec3 target_position_;
    int step_count_;
    double time_;
    std::vector<Entity> entities_;
    Entity* find_entity(const std::string& id);
};

}  // namespace synthia
