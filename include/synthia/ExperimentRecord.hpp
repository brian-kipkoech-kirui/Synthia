#pragma once

#include <string>
#include <vector>

#include "synthia/Action.hpp"
#include "synthia/Observation.hpp"
#include "synthia/Entity.hpp"

namespace synthia {

struct StepRecord {
    int step = 0;
    Observation observation;
    Action action;
    std::vector<Entity> world_state;
    bool success = false;
    double reward = 0.0;
    std::string message;
};

struct Episode {
    std::string episode_id;
    std::string world_seed;
    std::string scenario_id;
    std::string agent_id;
    std::vector<StepRecord> steps;
    std::string final_status;
    double final_reward = 0.0;
};

class ExperimentRecord {
public:
    explicit ExperimentRecord(std::string scenario_id);

    void add_step(const StepRecord& record);
    const std::vector<StepRecord>& steps() const;
    Episode finalize_episode(const std::string& agent_id, const std::string& world_seed) const;
    std::string summary() const;

private:
    std::string scenario_id_;
    std::vector<StepRecord> steps_;
};

}  // namespace synthia
