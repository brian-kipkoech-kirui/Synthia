#include "synthia/World.hpp"

#include <cmath>
#include <sstream>

namespace synthia {

namespace {
std::string make_world_id(const std::string& name, std::uint32_t seed) {
    std::ostringstream oss;
    oss << name << "_" << seed;
    return oss.str();
}
}  // namespace

World::World(const WorldConfig& config)
    : world_id_(make_world_id(config.name, config.seed)),
      seed_(config.seed),
      config_(config),
      start_position_(config.start_position),
      target_position_(config.target_position),
      step_count_(0),
      time_(0.0),
      entities_() {
    Entity start{"agent_0", "agent", start_position_, {0.0, 0.0, 0.0}, 0.0};
    entities_.push_back(start);

    for (int i = 0; i < 5; ++i) {
        Entity obstacle{"obstacle_" + std::to_string(i), "obstacle",
                       {static_cast<double>((i + 1) * 10), 0.0, static_cast<double>((i + 2) * 5)},
                       {0.0, 0.0, 0.0}, 0.0};
        entities_.push_back(obstacle);
    }
}

const std::string& World::world_id() const { return world_id_; }
std::uint32_t World::seed() const { return seed_; }
int World::width() const { return config_.width; }
int World::height() const { return config_.height; }
int World::step_count() const { return step_count_; }
double World::time() const { return time_; }
const std::vector<Entity>& World::entities() const { return entities_; }
const Vec3& World::start_position() const { return start_position_; }
const Vec3& World::target_position() const { return target_position_; }

void World::add_entity(const Entity& entity) { entities_.push_back(entity); }

void World::set_agent_position(const Vec3& position) {
    for (auto& entity : entities_) {
        if (entity.type == "agent") {
            entity.position = position;
            return;
        }
    }
}

void World::tick(double dt) {
    time_ += dt;
    ++step_count_;
}

Entity* World::find_entity(const std::string& id) {
    for (auto& entity : entities_) {
        if (entity.id == id) {
            return &entity;
        }
    }
    return nullptr;
}

SimulationStep World::step(const std::string& agent_id, const Action& action) {
    Entity* agent = find_entity(agent_id);
    if (agent == nullptr) {
        return {false, true, -1.0, "agent not found"};
    }

    switch (action.type) {
        case ActionType::MoveForward:
            agent->position.z += action.magnitude;
            break;
        case ActionType::MoveBackward:
            agent->position.z -= action.magnitude;
            break;
        case ActionType::TurnLeft:
            agent->heading -= action.magnitude;
            break;
        case ActionType::TurnRight:
            agent->heading += action.magnitude;
            break;
        case ActionType::Wait:
            break;
        default:
            break;
    }

    tick();

    const double dx = agent->position.x - target_position_.x;
    const double dy = agent->position.y - target_position_.y;
    const double dz = agent->position.z - target_position_.z;
    const double distance_to_target = std::sqrt(dx * dx + dy * dy + dz * dz);
    const bool reached_goal = (distance_to_target <= 0.5);
    if (reached_goal) {
        return {true, true, 10.0, "agent reached goal"};
    }

    return {false, false, 0.0, "step executed"};
}

void World::reset() {
    step_count_ = 0;
    time_ = 0.0;
    entities_.clear();
    Entity start{"agent_0", "agent", start_position_, {0.0, 0.0, 0.0}, 0.0};
    entities_.push_back(start);
}

}  // namespace synthia
