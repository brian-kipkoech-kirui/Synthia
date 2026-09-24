#include "synthia/knowledge/Observation.hpp"

#include <cassert>

int main() {
    synthia::knowledge::Observation observation;
    observation.id = "obs_001";
    observation.timestamp = 101;
    observation.source_id = "src_001";
    observation.content = "Alpha announced expansion";
    observation.data_type = "news";
    observation.confidence = 0.92;

    assert(observation.id == "obs_001");
    assert(observation.data_type == "news");
    assert(observation.confidence > 0.9);
    return 0;
}
