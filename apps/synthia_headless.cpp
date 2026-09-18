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
        return observation;
    }

    synthia::Action decide(const synthia::Observation& observation) override {
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

    synthia::WorldConfig config{"multi_agent_nav", static_cast<unsigned int>(seed), 12, 12, 10, "urban"};
    config.start_position = {0.0, 0.0, 0.0};
    config.target_position = {3.0, 0.0, 3.0};

    synthia::World world(config);
    world.add_agent("agent_0", {0.0, 0.0, 0.0}, {3.0, 0.0, 3.0});
    world.add_agent("agent_1", {0.0, 0.0, 1.0}, {2.0, 0.0, 2.0});

    TargetAgent agent_0("agent_0", {3.0, 0.0, 3.0});
    TargetAgent agent_1("agent_1", {2.0, 0.0, 2.0});
    std::vector<std::reference_wrapper<synthia::Agent>> agents = {agent_0, agent_1};

    synthia::Scenario scenario = make_multi_agent_nav_scenario(seed);
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
