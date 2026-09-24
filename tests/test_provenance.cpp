#include "synthia/knowledge/Provenance.hpp"

#include <cassert>

int main() {
    synthia::knowledge::Provenance provenance;
    provenance.observation_id = "obs_001";
    provenance.source_id = "src_news";
    provenance.observed_at = 200;
    provenance.confidence = 0.88;
    provenance.notes = "verified from primary report";

    assert(provenance.observation_id == "obs_001");
    assert(provenance.source_id == "src_news");
    assert(provenance.confidence == 0.88);
    return 0;
}
