#include "synthia/Task.hpp"

#include <cassert>

int main() {
    synthia::Task task;
    task.id = "task_1";
    task.description = "navigate to goal";
    task.status = synthia::TaskStatus::Pending;
    task.max_steps = 10;
    task.reward = 5.0;

    assert(task.status == synthia::TaskStatus::Pending);
    task.status = synthia::TaskStatus::Running;
    assert(task.status == synthia::TaskStatus::Running);
    task.status = synthia::TaskStatus::Success;
    assert(task.status == synthia::TaskStatus::Success);

    return 0;
}
