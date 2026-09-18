#include "synthia/ExperimentRecord.hpp"

#include <sstream>

namespace synthia {

ExperimentRecord::ExperimentRecord(std::string scenario_id, std::string world_id, std::string seed)
    : scenario_id_(std::move(scenario_id)),
      world_id_(std::move(world_id)),
      seed_(std::move(seed)) {}

void ExperimentRecord::set_world_info(std::string world_id, std::string seed) {
    world_id_ = std::move(world_id);
    seed_ = std::move(seed);
}

void ExperimentRecord::set_initial_state(const std::vector<Entity>& state) {
    initial_state_ = state;
}

void ExperimentRecord::add_step(const StepRecord& record) {
    steps_.push_back(record);
}

const std::vector<StepRecord>& ExperimentRecord::steps() const {
    return steps_;
}

const std::vector<Entity>& ExperimentRecord::initial_state() const {
    return initial_state_;
}

const std::string& ExperimentRecord::scenario_id() const {
    return scenario_id_;
}

const std::string& ExperimentRecord::world_id() const {
    return world_id_;
}

const std::string& ExperimentRecord::seed() const {
    return seed_;
}

Episode ExperimentRecord::finalize_episode(const std::string& agent_id, const std::string& world_seed) const {
    Episode episode;
    episode.episode_id = "ep_" + scenario_id_ + "_" + agent_id;
    episode.world_seed = world_seed.empty() ? seed_ : world_seed;
    episode.scenario_id = scenario_id_;
    episode.agent_id = agent_id;
    episode.steps = steps_;

    if (!steps_.empty()) {
        episode.final_status = steps_.back().success ? "SUCCESS" : (steps_.back().message == "step executed" ? "RUNNING" : "FAILURE");
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
