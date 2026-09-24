#pragma once

#include <string>
#include <vector>

#include "synthia/Action.hpp"
#include "synthia/Observation.hpp"
#include "synthia/Entity.hpp"
#include "synthia/World.hpp"

namespace synthia {

struct AgentStepRecord {
    std::string agent_id;
    Observation observation;
    Action action;
    bool success = false;
    double reward = 0.0;
    std::string message;
};

struct StepRecord {
    int step = 0;
    int simulation_time = 0;
    Observation observation;
    Action action;
    std::vector<Entity> world_state;
    std::vector<AgentStepRecord> agent_steps;
    std::vector<WorldEvent> events;
    bool success = false;
    bool done = false;
    double reward = 0.0;
    std::string message;
};

struct Episode {
    std::string episode_id;
    std::string world_seed;
    std::string scenario_id;
    std::string agent_id;
    std::vector<Entity> initial_state;
    std::vector<StepRecord> steps;
    std::string final_status;
    double final_reward = 0.0;
};

class ExperimentRecord {
public:
    explicit ExperimentRecord(std::string scenario_id, std::string world_id = "", std::string seed = "");

    void set_world_info(std::string world_id, std::string seed);
    void set_initial_state(const std::vector<Entity>& state);
    void add_step(const StepRecord& record);
    const std::vector<StepRecord>& steps() const;
    std::vector<StepRecord>& steps();
    const std::vector<Entity>& initial_state() const;
    const std::string& scenario_id() const;
    const std::string& world_id() const;
    const std::string& seed() const;
    Episode finalize_episode(const std::string& agent_id, const std::string& world_seed) const;
    std::string summary() const;

private:
    std::string scenario_id_;
    std::string world_id_;
    std::string seed_;
    std::vector<Entity> initial_state_;
    std::vector<StepRecord> steps_;
};

}  // namespace synthia
