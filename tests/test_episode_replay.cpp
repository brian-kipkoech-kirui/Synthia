#include "synthia/Action.hpp"
#include "synthia/ExperimentRecord.hpp"
#include "synthia/Observation.hpp"

#include <cassert>

int main() {
    synthia::ExperimentRecord record("delivery_v1");

    synthia::Observation obs;
    obs.agent_id = "agent_1";
    obs.goal = "navigate";
    obs.agent_position = {0.0, 0.0, 0.0};
    obs.target_position = {0.0, 0.0, 5.0};
    obs.distance_to_target = 5.0;
    obs.heading = 0.0;
    obs.available_actions = {synthia::ActionType::MoveForward, synthia::ActionType::Wait};

    synthia::StepRecord step;
    step.step = 0;
    step.observation = obs;
    step.action = {synthia::ActionType::MoveForward, "", 1.0};
    step.success = true;
    step.reward = 10.0;
    step.message = "success";

    record.add_step(step);
    const auto episode = record.finalize_episode("agent_1", "827194");

    assert(episode.scenario_id == "delivery_v1");
    assert(episode.world_seed == "827194");
    assert(episode.steps.size() == 1);
    assert(episode.final_status == "SUCCESS");

    return 0;
}
