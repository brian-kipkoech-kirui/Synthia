#include "synthia/world.hpp"

#include <iostream>

int main() {
    synthia::WorldConfig config{"demo_world", 1234, "urban"};
    synthia::World world(config);

    std::cout << "World ID: " << world.world_id() << "\n";
    std::cout << "Seed: " << world.seed() << "\n";
    std::cout << "Entities: " << world.entities().size() << "\n";
    std::cout << "Weather: rain=" << world.weather().rain
              << ", wind=" << world.weather().wind_speed << "\n";

    world.tick(1.0);
    std::cout << "Time: " << world.time() << "\n";
    return 0;
}
