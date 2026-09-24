#include "synthia/ScenarioRunner.hpp"

#include <cmath>
#include <sstream>

namespace synthia {

namespace {

std::string make_episode_id(const std::string& scenario_id, const std::string& seed, const std::string& agent_id) {
    std::ostringstream oss;
    oss << "ep_" << scenario_id << "_s" << seed << "_" << agent_id;
    return oss.str();
}

}  // namespace

ScenarioRunner::ScenarioRunner(World& world, Agent& agent, const Task& task, const Scenario& scenario)
    : world_(world),
      agents_({agent}),
      task_(task),
      scenario_(scenario),
      record_(scenario.id, world.world_id(), std::to_string(world.seed())) {
    record_.set_world_info(world.world_id(), std::to_string(world.seed()));
}

ScenarioRunner::ScenarioRunner(World& world, const std::vector<std::reference_wrapper<Agent>>& agents, const Task& task, const Scenario& scenario)
    : world_(world),
      agents_(agents),
      task_(task),
      scenario_(scenario),
      record_(scenario.id, world.world_id(), std::to_string(world.seed())) {
    record_.set_world_info(world.world_id(), std::to_string(world.seed()));
}

const ExperimentRecord& ScenarioRunner::record() const {
    return record_;
}

EpisodeResult ScenarioRunner::run() {
    record_.set_world_info(world_.world_id(), std::to_string(world_.seed()));

    EpisodeResult result;
    result.scenario_id = scenario_.id;
    result.world_id = world_.world_id();
    result.seed = std::to_string(world_.seed());
    result.episode_id = make_episode_id(scenario_.id, result.seed, agents_.empty() ? "unknown" : agents_[0].get().id());

    world_.reset();
    record_.set_initial_state(world_.entities());

    Task local_task = task_;
    local_task.status = TaskStatus::Running;
    local_task.completion_reason = "episode_started";

    const int total_steps = scenario_.steps.empty()
        ? std::max(0, scenario_.task.max_steps)
        : std::max(std::max(0, scenario_.task.max_steps), static_cast<int>(scenario_.steps.size()));

    for (int step_index = 0; step_index < total_steps; ++step_index) {
        StepRecord step_record;
        step_record.step = step_index;

        std::vector<AgentStepRecord> agent_steps;
        for (auto& agent_ref : agents_) {
            Agent& agent = agent_ref.get();
            Observation observation = agent.observe(world_);
            Action action = agent.decide(observation);
            const auto sim = world_.step(agent.id(), action);

            AgentStepRecord agent_step;
            agent_step.agent_id = agent.id();
            agent_step.observation = observation;
            agent_step.action = action;
            agent_step.reward = sim.reward;
            agent_step.success = sim.success;
            agent_step.message = sim.message;
            agent_steps.push_back(agent_step);

            result.total_reward += sim.reward;
            if (step_record.observation.agent_id.empty()) {
                step_record.observation = observation;
                step_record.action = action;
            }
            step_record.reward += sim.reward;
            step_record.success = step_record.success || sim.success;
            step_record.done = step_record.done || sim.done;
            step_record.message = sim.message;
            step_record.agent_steps = agent_steps;
        }

        step_record.simulation_time = world_.simulation_time();
        step_record.world_state = world_.entities();
        step_record.events = world_.events();

        record_.add_step(step_record);
        result.steps = step_index + 1;

        bool resources_complete = false;
        if (scenario_.task.success_condition == "all_resources_collected") {
            resources_complete = true;
            for (const auto& resource : world_.resources()) {
                if (resource.active) {
                    resources_complete = false;
                    break;
                }
            }
        }

        const bool survival_complete = scenario_.task.success_condition == "survival_ticks" &&
            (step_index + 1) >= scenario_.task.max_steps;
        const bool task_success = (scenario_.task.success_condition == "goal_reached") ? step_record.success :
            ((scenario_.task.success_condition == "all_resources_collected") ? resources_complete :
            ((scenario_.task.success_condition == "survival_ticks") ? survival_complete : step_record.success));
        const bool task_done = step_record.done;

        local_task.status = local_task.evaluate(step_index, task_success, task_done);
        local_task.completion_reason = task_success ? "goal_reached" : (task_done ? "world_failure" : "step_completed");

        if (local_task.status == TaskStatus::Success) {
            result.termination = EpisodeTermination::Success;
            result.final_status = "SUCCESS";
            step_record.message = "success";
            if (!record_.steps().empty()) {
                record_.steps().back().message = step_record.message;
                record_.steps().back().success = step_record.success;
                record_.steps().back().done = step_record.done;
                record_.steps().back().reward = step_record.reward;
            }
            break;
        }

        if (local_task.status == TaskStatus::Timeout) {
            result.termination = EpisodeTermination::Timeout;
            result.final_status = "TIMEOUT";
            step_record.message = "timeout";
            if (!record_.steps().empty()) {
                record_.steps().back().message = step_record.message;
                record_.steps().back().success = step_record.success;
                record_.steps().back().done = step_record.done;
                record_.steps().back().reward = step_record.reward;
            }
            break;
        }

        if (local_task.status == TaskStatus::Failed) {
            result.termination = EpisodeTermination::Failure;
            result.final_status = "FAILURE";
            step_record.message = "failed";
            if (!record_.steps().empty()) {
                record_.steps().back().message = step_record.message;
                record_.steps().back().success = step_record.success;
                record_.steps().back().done = step_record.done;
                record_.steps().back().reward = step_record.reward;
            }
            break;
        }

        if (!record_.steps().empty()) {
            record_.steps().back().message = step_record.message;
            record_.steps().back().success = step_record.success;
            record_.steps().back().done = step_record.done;
            record_.steps().back().reward = step_record.reward;
        }
    }

    if (result.steps == 0) {
        result.termination = EpisodeTermination::Failure;
        result.final_status = "FAILURE";
    }

    return result;
}

ReplayResult replay(const ExperimentRecord& record) {
    ReplayResult result;
    result.scenario_id = record.scenario_id();
    result.seed = record.seed();
    result.world_id = record.world_id();
    result.termination = EpisodeTermination::Failure;

    for (const auto& step : record.steps()) {
        ReplayStep replay_step;
        replay_step.step = step.step;
        replay_step.reward = step.reward;
        replay_step.success = step.success;
        replay_step.status = step.message;
        for (const auto& a : step.agent_steps) {
            ReplayAgentStep r;
            r.agent_id = a.agent_id;
            r.observation = a.observation;
            r.action = a.action;
            r.reward = a.reward;
            r.success = a.success;
            r.status = a.message;
            replay_step.agent_steps.push_back(r);
            result.total_reward += a.reward;
        }
        result.steps.push_back(replay_step);

        if (step.success) {
            result.termination = EpisodeTermination::Success;
        } else if (step.message == "timeout") {
            result.termination = EpisodeTermination::Timeout;
        } else if (step.message == "failed" || step.message == "invalid action" || step.message == "insufficient energy") {
            result.termination = EpisodeTermination::Failure;
        }
    }

    if (result.steps.empty()) {
        result.termination = EpisodeTermination::Failure;
    }

    return result;
}

}  // namespace synthia
