#pragma once

#include <string>
#include <vector>

namespace synthia {

enum class TaskStatus {
    Pending,
    Running,
    Success,
    Failed,
    Timeout
};

struct Task {
    std::string id;
    std::string description;
    TaskStatus status = TaskStatus::Pending;
    std::string start_condition;
    std::string success_condition;
    std::string failure_condition;
    int max_steps = 100;
    double reward = 0.0;
    std::string completion_reason;

    TaskStatus evaluate(int step_index, bool success, bool done) const {
        if (success) {
            return TaskStatus::Success;
        }
        if (done && !success) {
            return TaskStatus::Failed;
        }
        if (step_index + 1 >= max_steps) {
            return TaskStatus::Timeout;
        }
        return TaskStatus::Running;
    }
};

}  // namespace synthia
