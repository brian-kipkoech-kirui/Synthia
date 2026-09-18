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
    agent_targets_["agent_0"] = target_position_;

    for (int i = 0; i < 5; ++i) {
        Entity obstacle{"obstacle_" + std::to_string(i), "obstacle",
                       {static_cast<double>((i + 1) * 10), 0.0, static_cast<double>((i + 2) * 5)},
                       {0.0, 0.0, 0.0}, 0.0};
        entities_.push_back(obstacle);
    }

    initial_entities_ = entities_;
    initial_agent_targets_ = agent_targets_;
}

const std::string& World::world_id() const { return world_id_; }
std::uint32_t World::seed() const { return seed_; }
int World::width() const { return config_.width; }
int World::height() const { return config_.height; }
int World::step_count() const { return step_count_; }
int World::simulation_time() const { return simulation_time_; }
double World::time() const { return time_; }
const std::vector<Entity>& World::entities() const { return entities_; }
const Vec3& World::start_position() const { return start_position_; }
const Vec3& World::target_position() const { return target_position_; }
std::vector<std::string> World::agent_ids() const {
    std::vector<std::string> ids;
    for (const auto& entity : entities_) {
        if (entity.type == "agent") {
            ids.push_back(entity.id);
        }
    }
    return ids;
}
bool World::has_agent(const std::string& agent_id) const {
    return find_entity(agent_id) != nullptr;
}
const Vec3& World::agent_position(const std::string& agent_id) const {
    static const Vec3 empty{0.0, 0.0, 0.0};
    const auto* entity = find_entity(agent_id);
    return entity ? entity->position : empty;
}
const Vec3& World::agent_target_position(const std::string& agent_id) const {
    static const Vec3 empty{0.0, 0.0, 0.0};
    auto it = agent_targets_.find(agent_id);
    return it != agent_targets_.end() ? it->second : empty;
}

std::vector<Entity> World::resources() const {
    std::vector<Entity> result;
    for (const auto& entity : entities_) {
        if (entity.type == "resource") {
            result.push_back(entity);
        }
    }
    return result;
}

std::vector<WorldEvent> World::events() const {
    return events_;
}

WorldState World::snapshot() const {
    WorldState state;
    state.simulation_time = simulation_time_;
    state.entities = entities_;
    state.events = events_;
    return state;
}

bool World::is_valid_position(const Vec3& position) const {
    return position.x >= 0.0 && position.x < static_cast<double>(config_.width) &&
           position.y >= 0.0 && position.y < static_cast<double>(config_.height) &&
           position.z >= 0.0 && position.z < static_cast<double>(config_.height);
}

bool World::is_occupied(const Vec3& position) const {
    for (const auto& entity : entities_) {
        if (entity.active && entity.position == position) {
            return true;
        }
    }
    return false;
}

void World::add_event(const WorldEvent& event) {
    events_.push_back(event);
}

void World::add_resource(const std::string& resource_id, const Vec3& position, const std::string& kind) {
    Entity resource{resource_id, "resource", position, {0.0, 0.0, 0.0}, 0.0, true, kind};
    entities_.push_back(resource);
    initial_entities_.push_back(resource);
    add_event({resource_id, "resource_spawned", "world", position, kind});
}

void World::add_entity(const Entity& entity) {
    entities_.push_back(entity);
    if (entity.type == "agent") {
        initial_entities_.push_back(entity);
    }
}

void World::add_agent(const std::string& agent_id, const Vec3& start_position, const Vec3& target_position) {
    Entity agent{agent_id, "agent", start_position, {0.0, 0.0, 0.0}, 0.0};
    entities_.push_back(agent);
    agent_targets_[agent_id] = target_position;
    initial_entities_.push_back(agent);
    initial_agent_targets_[agent_id] = target_position;
}

void World::set_agent_position(const std::string& agent_id, const Vec3& position) {
    Entity* entity = find_entity(agent_id);
    if (entity != nullptr) {
        entity->position = position;
    }
}

void World::set_agent_target(const std::string& agent_id, const Vec3& target_position) {
    agent_targets_[agent_id] = target_position;
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

const Entity* World::find_entity(const std::string& id) const {
    for (const auto& entity : entities_) {
        if (entity.id == id) {
            return &entity;
        }
    }
    return nullptr;
}

bool World::apply_action(const std::string& agent_id, const Action& action) {
    Entity* agent = find_entity(agent_id);
    if (agent == nullptr) {
        return false;
    }

    if (action.type == ActionType::Pickup) {
        for (auto& entity : entities_) {
            if (entity.id == action.target_id && entity.type == "resource" && entity.active) {
                const auto dx = entity.position.x - agent->position.x;
                const auto dy = entity.position.y - agent->position.y;
                const auto dz = entity.position.z - agent->position.z;
                const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                if (distance <= 1.5) {
                    entity.active = false;
                    add_event({action.target_id, "resource_collected", agent_id, entity.position, "resource collected"});
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    Vec3 next = agent->position;
    switch (action.type) {
        case ActionType::MoveForward:
            next.z += action.magnitude;
            break;
        case ActionType::MoveBackward:
            next.z -= action.magnitude;
            break;
        case ActionType::TurnLeft:
            agent->heading -= action.magnitude;
            return true;
        case ActionType::TurnRight:
            agent->heading += action.magnitude;
            return true;
        case ActionType::MoveLeft:
            next.x -= action.magnitude;
            break;
        case ActionType::MoveRight:
            next.x += action.magnitude;
            break;
        case ActionType::MoveUp:
            next.z += action.magnitude;
            break;
        case ActionType::MoveDown:
            next.z -= action.magnitude;
            break;
        case ActionType::Wait:
            return true;
        default:
            return false;
    }

    if (!is_valid_position(next) || is_occupied(next)) {
        return false;
    }

    agent->position = next;
    return true;
}

SimulationStep World::step(const std::string& agent_id, const Action& action) {
    Entity* agent = find_entity(agent_id);
    if (agent == nullptr) {
        for (auto& entity : entities_) {
            if (entity.type == "agent") {
                agent = &entity;
                agent->id = agent_id;
                break;
            }
        }
    }
    if (agent == nullptr) {
        return {false, true, -1.0, "agent not found"};
    }

    if (!apply_action(agent_id, action)) {
        if (action.type == ActionType::Pickup) {
            return {false, true, -1.0, "resource unavailable"};
        }
        return {false, true, -1.0, "invalid action"};
    }

    ++simulation_time_;
    tick();

    const auto it = agent_targets_.find(agent_id);
    const Vec3 target = it != agent_targets_.end() ? it->second : target_position_;
    const double dx = agent->position.x - target.x;
    const double dy = agent->position.y - target.y;
    const double dz = agent->position.z - target.z;
    const double distance_to_target = std::sqrt(dx * dx + dy * dy + dz * dz);
    const bool reached_goal = (distance_to_target <= 0.5);
    if (action.type == ActionType::Pickup) {
        for (const auto& entity : entities_) {
            if (entity.id == action.target_id && entity.type == "resource" && !entity.active) {
                return {true, true, 25.0, "resource collected"};
            }
        }
    }

    if (reached_goal) {
        return {true, true, 10.0, "agent reached goal"};
    }

    return {false, false, 0.0, "step executed"};
}

void World::reset() {
    step_count_ = 0;
    simulation_time_ = 0;
    time_ = 0.0;
    entities_ = initial_entities_;
    events_ = initial_events_;
    agent_targets_ = initial_agent_targets_;
}

}  // namespace synthia
