#pragma once

#include <string>

namespace synthia {

enum class ActionType {
    MoveForward,
    MoveBackward,
    TurnLeft,
    TurnRight,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Pickup,
    Drop,
    Interact,
    Wait
};

struct Action {
    ActionType type = ActionType::Wait;
    std::string target_id;
    double magnitude = 1.0;
};

}  // namespace synthia
