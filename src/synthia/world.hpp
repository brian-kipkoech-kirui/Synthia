#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace synthia {

struct Vec3 {
    double x;
    double y;
    double z;
};

struct Entity {
    std::string id;
    std::string type;
    Vec3 position;
    Vec3 velocity;
    double heading;
};

struct WeatherState {
    double temperature;
    double rain;
    double wind_speed;
};

struct WorldConfig {
    std::string name;
    std::uint32_t seed;
    std::string climate;
    int terrain_size = 100;
    int road_count = 8;
    int building_count = 20;
};

class World {
public:
    explicit World(const WorldConfig& config);

    const std::string& world_id() const;
    std::uint32_t seed() const;
    double time() const;

    void tick(double dt);
    const std::vector<Entity>& entities() const;
    const WeatherState& weather() const;

private:
    std::string world_id_;
    std::uint32_t seed_;
    std::string name_;
    std::string climate_;
    double time_;
    WeatherState weather_;
    std::vector<Entity> entities_;
};

}  // namespace synthia
