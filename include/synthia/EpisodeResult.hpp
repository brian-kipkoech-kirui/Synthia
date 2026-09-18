#pragma once

#include <string>

namespace synthia {

enum class EpisodeTermination {
    Success,
    Failure,
    Timeout,
    Aborted,
    Error
};

inline std::string to_string(EpisodeTermination termination) {
    switch (termination) {
        case EpisodeTermination::Success:
            return "SUCCESS";
        case EpisodeTermination::Failure:
            return "FAILURE";
        case EpisodeTermination::Timeout:
            return "TIMEOUT";
        case EpisodeTermination::Aborted:
            return "ABORTED";
        case EpisodeTermination::Error:
            return "ERROR";
    }
    return "UNKNOWN";
}

struct EpisodeResult {
    std::string episode_id;
    std::string scenario_id;
    std::string world_id;
    std::string seed;
    int steps = 0;
    EpisodeTermination termination = EpisodeTermination::Failure;
    double total_reward = 0.0;
    std::string final_status;
};

}  // namespace synthia
