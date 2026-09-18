#include "synthia/ExperimentRecord.hpp"

#include <sstream>

namespace synthia {

ExperimentRecord::ExperimentRecord(std::string scenario_id)
    : scenario_id_(std::move(scenario_id)) {}

void ExperimentRecord::add_step(const StepRecord& record) {
    steps_.push_back(record);
}

const std::vector<StepRecord>& ExperimentRecord::steps() const {
    return steps_;
}

Episode ExperimentRecord::finalize_episode(const std::string& agent_id, const std::string& world_seed) const {
    Episode episode;
    episode.episode_id = "ep_" + scenario_id_ + "_" + agent_id;
    episode.world_seed = world_seed;
    episode.scenario_id = scenario_id_;
    episode.agent_id = agent_id;
    episode.steps = steps_;

    if (!steps_.empty()) {
        episode.final_status = steps_.back().success ? "SUCCESS" : "RUNNING";
        episode.final_reward = steps_.back().reward;
    } else {
        episode.final_status = "PENDING";
    }

    return episode;
}

std::string ExperimentRecord::summary() const {
    std::ostringstream oss;
    oss << "scenario=" << scenario_id_ << "; steps=" << steps_.size();
    return oss.str();
}

}  // namespace synthia
