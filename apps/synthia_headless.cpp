#include "synthia/Action.hpp"
#include "synthia/Agent.hpp"
#include "synthia/ExperimentRecord.hpp"
#include "synthia/ScenarioRunner.hpp"
#include "synthia/World.hpp"
#include "synthia/WorldConfig.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string action_name(synthia::ActionType type) {
    switch (type) {
        case synthia::ActionType::MoveForward: return "MOVE_FORWARD";
        case synthia::ActionType::MoveBackward: return "MOVE_BACKWARD";
        case synthia::ActionType::TurnLeft: return "TURN_LEFT";
        case synthia::ActionType::TurnRight: return "TURN_RIGHT";
        case synthia::ActionType::MoveLeft: return "MOVE_LEFT";
        case synthia::ActionType::MoveRight: return "MOVE_RIGHT";
        case synthia::ActionType::MoveUp: return "MOVE_UP";
        case synthia::ActionType::MoveDown: return "MOVE_DOWN";
        case synthia::ActionType::Pickup: return "PICKUP";
        case synthia::ActionType::Drop: return "DROP";
        case synthia::ActionType::Interact: return "INTERACT";
        case synthia::ActionType::Wait: return "WAIT";
        default: return "UNKNOWN";
    }
}

std::string termination_name(synthia::EpisodeTermination termination) {
    switch (termination) {
        case synthia::EpisodeTermination::Success: return "SUCCESS";
        case synthia::EpisodeTermination::Failure: return "FAILURE";
        case synthia::EpisodeTermination::Timeout: return "TIMEOUT";
        case synthia::EpisodeTermination::Aborted: return "ABORTED";
        case synthia::EpisodeTermination::Error: return "ERROR";
    }
    return "UNKNOWN";
}

class TargetAgent : public synthia::Agent {
public:
    TargetAgent(std::string id, synthia::Vec3 target)
        : synthia::Agent(std::move(id), "navigate"), target_(std::move(target)) {}

    synthia::Observation observe(const synthia::World& world) override {
        synthia::Observation observation;
        observation.agent_id = id_;
        observation.goal = goal_;
        observation.status = "running";
        observation.available_actions = {
            synthia::ActionType::MoveLeft,
            synthia::ActionType::MoveRight,
            synthia::ActionType::MoveUp,
            synthia::ActionType::MoveDown,
            synthia::ActionType::Wait
        };
        observation.agent_position = world.agent_position(id_);
        observation.target_position = target_;
        const auto dx = observation.agent_position.x - target_.x;
        const auto dy = observation.agent_position.y - target_.y;
        const auto dz = observation.agent_position.z - target_.z;
        observation.distance_to_target = std::sqrt(dx * dx + dy * dy + dz * dz);
        observation.heading = 0.0;
        observation.energy = world.agent_energy(id_);
        observation.simulation_time = world.simulation_time();
        observation.terrain = world.terrain_at(observation.agent_position);
        observation.environment = world.environment();
        for (const auto& entity : world.entities()) {
            if (entity.type == "resource" && entity.active && world.distance(entity.position, observation.agent_position) <= 1.5) {
                observation.nearby_entities.push_back(entity);
                observation.nearby_resource_ids.push_back(entity.id);
            }
        }
        return observation;
    }

    synthia::Action decide(const synthia::Observation& observation) override {
        if (!observation.nearby_resource_ids.empty()) {
            return {synthia::ActionType::Pickup, observation.nearby_resource_ids.front(), 1.0};
        }
        if (observation.distance_to_target > 0.5) {
            if (observation.agent_position.x > observation.target_position.x) {
                return {synthia::ActionType::MoveLeft, "", 1.0};
            }
            if (observation.agent_position.x < observation.target_position.x) {
                return {synthia::ActionType::MoveRight, "", 1.0};
            }
            if (observation.agent_position.z > observation.target_position.z) {
                return {synthia::ActionType::MoveDown, "", 1.0};
            }
            return {synthia::ActionType::MoveUp, "", 1.0};
        }
        return {synthia::ActionType::Wait, "", 0.0};
    }

private:
    synthia::Vec3 target_;
};

class SurvivalAgent : public TargetAgent {
public:
    SurvivalAgent(std::string id, synthia::Vec3 target)
        : TargetAgent(std::move(id), std::move(target)) {}
};

synthia::Scenario make_multi_agent_nav_scenario(int seed) {
    synthia::Scenario scenario;
    scenario.id = "multi_agent_nav";
    scenario.name = "multi_agent_navigation";
    scenario.description = "two agents navigate to their targets in a shared world";
    scenario.seed = seed;
    scenario.task.id = "multi_agent_task";
    scenario.task.description = "all agents reach their targets";
    scenario.task.max_steps = 8;
    scenario.task.success_condition = "all_agents_reached_targets";
    scenario.task.failure_condition = "failed";
    return scenario;
}

synthia::Scenario make_survival_scenario(int seed) {
    synthia::Scenario scenario;
    scenario.id = "physics_survival";
    scenario.name = "physics_survival";
    scenario.description = "agents survive deterministic environmental physics";
    scenario.seed = seed;
    scenario.task.id = "survive_environment";
    scenario.task.description = "remain active while navigating hazards and collecting resources";
    scenario.task.max_steps = 8;
    scenario.task.success_condition = "survival_ticks";
    scenario.task.failure_condition = "failed";
    return scenario;
}

void write_record_file(const std::string& path, const synthia::ExperimentRecord& record) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "failed to write record: " << path << "\n";
        return;
    }
    out << "scenario=" << record.scenario_id() << "\n";
    out << "seed=" << record.seed() << "\n";
    out << "world_id=" << record.world_id() << "\n";
    out << "initial_agents=" << record.initial_state().size() << "\n";
    for (const auto& step : record.steps()) {
        out << "step=" << step.step << " reward=" << step.reward << " success=" << (step.success ? 1 : 0) << " done=" << (step.done ? 1 : 0) << " message=" << step.message << "\n";
        for (const auto& agent_step : step.agent_steps) {
            out << "agent=" << agent_step.agent_id << " action=" << action_name(agent_step.action.type)
                << " reward=" << agent_step.reward << " success=" << (agent_step.success ? 1 : 0) << "\n";
        }
    }
}

void replay_record_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "failed to open record: " << path << "\n";
        return;
    }
    std::string line;
    std::cout << "REPLAY " << path << "\n";
    while (std::getline(in, line)) {
        std::cout << line << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    int seed = 42;
    std::string scenario_name = "multi_agent_nav";
    std::string record_path;
    std::string replay_path;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--scenario" && i + 1 < argc) {
            scenario_name = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = std::stoi(argv[++i]);
        } else if (arg == "--record" && i + 1 < argc) {
            record_path = argv[++i];
        } else if (arg == "--replay" && i + 1 < argc) {
            replay_path = argv[++i];
        }
    }

    if (!replay_path.empty()) {
        replay_record_file(replay_path);
        return 0;
    }

    const bool survival = scenario_name == "physics_survival";
    const bool resources = scenario_name == "resource_collection";
    synthia::WorldConfig config{scenario_name, static_cast<unsigned int>(seed), survival ? 20 : 12, survival ? 20 : 12, survival ? 8 : 10, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {3.0, 0.0, 3.0};

    synthia::World world(config);
    if (survival) {
        world.add_agent("agent_0", {0.0, 0.0, 0.0}, {4.0, 0.0, 4.0});
        world.add_agent("agent_1", {0.0, 0.0, 2.0}, {4.0, 0.0, 6.0});
        world.add_agent("agent_2", {2.0, 0.0, 0.0}, {6.0, 0.0, 4.0});
        world.add_obstacle("wall_0", {2.0, 0.0, 1.0});
        world.add_obstacle("wall_1", {2.0, 0.0, 2.0});
        world.set_terrain({4.0, 0.0, 3.0}, synthia::TerrainType::Hazard);
        world.set_terrain({5.0, 0.0, 3.0}, synthia::TerrainType::Water);
        world.add_resource("resource_0", {3.0, 0.0, 3.0}, "energy_cell");
    } else {
        world.add_agent("agent_0", {0.0, 0.0, 0.0}, {3.0, 0.0, 3.0});
        world.add_agent("agent_1", {0.0, 0.0, 1.0}, {2.0, 0.0, 2.0});
        if (resources) {
            world.add_resource("resource_0", {3.0, 0.0, 3.0}, "energy_cell");
        }
    }

    TargetAgent agent_0("agent_0", survival ? synthia::Vec3{4.0, 0.0, 4.0} : synthia::Vec3{3.0, 0.0, 3.0});
    TargetAgent agent_1("agent_1", survival ? synthia::Vec3{4.0, 0.0, 6.0} : synthia::Vec3{2.0, 0.0, 2.0});
    SurvivalAgent agent_2("agent_2", {6.0, 0.0, 4.0});
    std::vector<std::reference_wrapper<synthia::Agent>> agents = survival ? std::vector<std::reference_wrapper<synthia::Agent>>{agent_0, agent_1, agent_2} : std::vector<std::reference_wrapper<synthia::Agent>>{agent_0, agent_1};

    synthia::Scenario scenario = survival ? make_survival_scenario(seed) : (resources ? make_survival_scenario(seed) : make_multi_agent_nav_scenario(seed));
    if (resources) {
        scenario.id = "resource_collection";
        scenario.task.success_condition = "all_resources_collected";
        scenario.task.max_steps = 10;
    }
    synthia::ScenarioRunner runner(world, agents, scenario.task, scenario);
    const auto result = runner.run();

    std::cout << "SYNTHIA\n";
    std::cout << "scenario=" << scenario_name << "\n";
    std::cout << "seed=" << seed << "\n";
    std::cout << "agents=" << agents.size() << "\n";

    const auto& record = runner.record();
    for (const auto& step : record.steps()) {
        std::cout << "step=" << step.step << "\n";
        for (const auto& agent_step : step.agent_steps) {
            std::cout << "  " << agent_step.agent_id << " action=" << action_name(agent_step.action.type)
                      << " reward=" << agent_step.reward << "\n";
        }
    }

    std::cout << "result=" << termination_name(result.termination) << "\n";
    std::cout << "steps=" << result.steps << "\n";

    if (!record_path.empty()) {
        write_record_file(record_path, record);
    }

    return 0;
}
