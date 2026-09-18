#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "synthia/Action.hpp"
#include "synthia/Entity.hpp"
#include "synthia/WorldConfig.hpp"

namespace synthia {

struct WorldEvent {
    std::string id;
    std::string type;
    std::string actor_id;
    Vec3 position;
    std::string message;
};

struct WorldState {
    int simulation_time = 0;
    std::vector<Entity> entities;
    std::vector<WorldEvent> events;
};

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
    int simulation_time() const;
    double time() const;
    const std::vector<Entity>& entities() const;
    const Vec3& start_position() const;
    const Vec3& target_position() const;
    std::vector<std::string> agent_ids() const;
    bool has_agent(const std::string& agent_id) const;
    const Vec3& agent_position(const std::string& agent_id) const;
    const Vec3& agent_target_position(const std::string& agent_id) const;
    const Entity* find_entity(const std::string& id) const;
    std::vector<Entity> resources() const;
    std::vector<WorldEvent> events() const;
    WorldState snapshot() const;
    bool is_valid_position(const Vec3& position) const;
    bool is_occupied(const Vec3& position) const;
    void add_event(const WorldEvent& event);
    void add_resource(const std::string& resource_id, const Vec3& position, const std::string& kind = "resource");
    void add_entity(const Entity& entity);
    void add_agent(const std::string& agent_id, const Vec3& start_position, const Vec3& target_position);
    void set_agent_position(const std::string& agent_id, const Vec3& position);
    void set_agent_target(const std::string& agent_id, const Vec3& target_position);
    void tick(double dt = 1.0);
    bool apply_action(const std::string& agent_id, const Action& action);
    SimulationStep step(const std::string& agent_id, const Action& action);
    void reset();

private:
    std::string world_id_;
    std::uint32_t seed_;
    WorldConfig config_;
    Vec3 start_position_;
    Vec3 target_position_;
    int step_count_;
    int simulation_time_;
    double time_;
    std::vector<Entity> entities_;
    std::vector<Entity> initial_entities_;
    std::vector<WorldEvent> events_;
    std::vector<WorldEvent> initial_events_;
    std::map<std::string, Vec3> agent_targets_;
    std::map<std::string, Vec3> initial_agent_targets_;
    Entity* find_entity(const std::string& id);
};

}  // namespace synthia
