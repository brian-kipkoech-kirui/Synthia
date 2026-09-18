#pragma once

#include <string>
#include <vector>

#include "synthia/Task.hpp"

namespace synthia {

struct Scenario {
    std::string id;
    std::string name;
    std::string description;
    Task task;
    std::vector<std::string> tags;
    int seed = 1;
    std::vector<std::string> steps;
};

}  // namespace synthia
