#include "synthia/world.hpp"

#include <sstream>
#include <iomanip>

namespace synthia {

namespace {
std::string make_world_id(const std::string& name, std::uint32_t seed) {
    std::ostringstream oss;
    oss << name << "_" << std::hex << seed;
    return oss.str();
}
}  // namespace

World::World(const WorldConfig& config)
    : world_id_(make_world_id(config.name, config.seed)),
      seed_(config.seed),
      name_(config.name),
      climate_(config.climate),
      time_(0.0),
      weather_{24.7, 0.0, 5.0},
      entities_() {
    Entity origin{"agent_0", "agent", {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 0.0};
    entities_.push_back(origin);

    for (int i = 1; i <= config.building_count; ++i) {
        Entity building{"building_" + std::to_string(i), "building",
                        {static_cast<double>((i * 7) % config.terrain_size), 0.0,
                         static_cast<double>((i * 11) % config.terrain_size)},
                        {0.0, 0.0, 0.0}, 0.0};
        entities_.push_back(building);
    }
}

const std::string& World::world_id() const {
    return world_id_;
}

std::uint32_t World::seed() const {
    return seed_;
}

double World::time() const {
    return time_;
}

void World::tick(double dt) {
    time_ += dt;
    weather_.rain = std::max(0.0, weather_.rain + 0.01 * dt);
    weather_.wind_speed += 0.05 * dt;

    for (auto& entity : entities_) {
        entity.position.x += entity.velocity.x * dt;
        entity.position.z += entity.velocity.z * dt;
    }
}

const std::vector<Entity>& World::entities() const {
    return entities_;
}

const WeatherState& World::weather() const {
    return weather_;
}

}  // namespace synthia
