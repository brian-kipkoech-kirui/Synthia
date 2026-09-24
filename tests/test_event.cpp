#include "synthia/knowledge/Event.hpp"

#include <cassert>

int main() {
    synthia::knowledge::Event event;
    event.id = "evt_001";
    event.timestamp = 200;
    event.event_type = "market_movement";
    event.description = "Alpha stock rose after expansion";
    event.entity_ids = {"entity_alpha", "entity_beta"};

    assert(event.event_type == "market_movement");
    assert(event.entity_ids.size() == 2);
    assert(event.description.find("Alpha") != std::string::npos);
    return 0;
}
