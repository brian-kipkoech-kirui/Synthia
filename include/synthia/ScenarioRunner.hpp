#pragma once

#include <functional>
#include <string>
#include <vector>

#include "synthia/Agent.hpp"
#include "synthia/EpisodeResult.hpp"
#include "synthia/ExperimentRecord.hpp"
#include "synthia/Scenario.hpp"
#include "synthia/Task.hpp"
#include "synthia/World.hpp"
#include "synthia/WorldConfig.hpp"

namespace synthia {

class ScenarioRunner {
public:
    ScenarioRunner(World& world, Agent& agent, const Task& task, const Scenario& scenario);
    ScenarioRunner(World& world, const std::vector<std::reference_wrapper<Agent>>& agents, const Task& task, const Scenario& scenario);

    EpisodeResult run();
    const ExperimentRecord& record() const;

private:
    World& world_;
    std::vector<std::reference_wrapper<Agent>> agents_;
    const Task& task_;
    const Scenario& scenario_;
    ExperimentRecord record_;
};

struct ReplayAgentStep {
    std::string agent_id;
    Observation observation;
    Action action;
    double reward = 0.0;
    bool success = false;
    std::string status;

    bool operator==(const ReplayAgentStep& other) const {
        return agent_id == other.agent_id &&
               observation.agent_id == other.observation.agent_id &&
               observation.goal == other.observation.goal &&
               observation.status == other.observation.status &&
               observation.agent_position == other.observation.agent_position &&
               observation.target_position == other.observation.target_position &&
               observation.distance_to_target == other.observation.distance_to_target &&
               observation.heading == other.observation.heading &&
               observation.available_actions == other.observation.available_actions &&
               observation.reward == other.observation.reward &&
               action.type == other.action.type &&
               action.target_id == other.action.target_id &&
               action.magnitude == other.action.magnitude &&
               reward == other.reward &&
               success == other.success &&
               status == other.status;
    }
};

struct ReplayStep {
    int step = 0;
    std::vector<ReplayAgentStep> agent_steps;
    double reward = 0.0;
    bool success = false;
    std::string status;

    bool operator==(const ReplayStep& other) const {
        return step == other.step &&
               agent_steps == other.agent_steps &&
               reward == other.reward &&
               success == other.success &&
               status == other.status;
    }
};

struct ReplayResult {
    std::string scenario_id;
    std::string world_id;
    std::string seed;
    std::vector<ReplayStep> steps;
    EpisodeTermination termination = EpisodeTermination::Failure;
    double total_reward = 0.0;
};

ReplayResult replay(const ExperimentRecord& record);

}  // namespace synthia
