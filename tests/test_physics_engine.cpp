#include "synthia/World.hpp"

#include <cassert>

int main() {
    synthia::WorldConfig config{"physics", 42, 5, 5, 10, "urban"};
    synthia::World world(config);
    world.add_agent("agent_a", {0.0, 0.0, 0.0}, {4.0, 0.0, 4.0});
    world.add_agent("agent_b", {1.0, 0.0, 1.0}, {4.0, 0.0, 4.0});
    world.add_obstacle("wall", {2.0, 0.0, 1.0});
    world.set_terrain({3.0, 0.0, 0.0}, synthia::TerrainType::Water);
    world.set_terrain({0.0, 0.0, 1.0}, synthia::TerrainType::Hazard);

    assert(world.distance({0.0, 0.0, 0.0}, {3.0, 0.0, 4.0}) == 5.0);
    assert(world.manhattan_distance({0.0, 0.0, 0.0}, {3.0, 0.0, 4.0}) == 7);
    assert(world.adjacent({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}));
    assert(world.terrain_at({2.0, 0.0, 1.0}) == synthia::TerrainType::Obstacle);

    const auto valid = world.step("agent_a", {synthia::ActionType::MoveUp, "", 1.0});
    assert(valid.message == "step executed");
    assert(world.agent_energy("agent_a") < 100.0);
    assert(!world.events().empty());

    const auto obstacle = world.step("agent_a", {synthia::ActionType::MoveRight, "", 2.0});
    assert(obstacle.message == "invalid action");
    const auto position = world.agent_position("agent_a");
    assert(position.x == 0.0 && position.y == 0.0 && position.z == 1.0);

    const auto boundary = world.step("agent_a", {synthia::ActionType::MoveLeft, "", 2.0});
    assert(boundary.message == "invalid action");
    const auto other_agent = world.step("agent_a", {synthia::ActionType::MoveRight, "", 1.0});
    assert(other_agent.message == "invalid action");

    const auto snapshot = world.snapshot();
    assert(snapshot.width == 5);
    assert(snapshot.height == 5);
    assert(snapshot.seed == 42);
    assert(snapshot.environment.weather == "wind" || snapshot.environment.weather == "rain" || snapshot.environment.weather == "heat" || snapshot.environment.weather == "clear");

    synthia::World same(config);
    same.add_agent("agent_a", {0.0, 0.0, 0.0}, {4.0, 0.0, 4.0});
    same.add_agent("agent_b", {1.0, 0.0, 1.0}, {4.0, 0.0, 4.0});
    same.add_obstacle("wall", {2.0, 0.0, 1.0});
    same.set_terrain({3.0, 0.0, 0.0}, synthia::TerrainType::Water);
    same.set_terrain({0.0, 0.0, 1.0}, synthia::TerrainType::Hazard);
    same.step("agent_a", {synthia::ActionType::MoveUp, "", 1.0});
    assert(same.snapshot().environment.weather == "rain");

    return 0;
}