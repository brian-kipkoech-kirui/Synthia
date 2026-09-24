#pragma once

#include "synthia/knowledge/WorldState.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace synthia {
namespace api {

struct DashboardWorld {
    synthia::knowledge::WorldState state;
    std::string engine_status = "RUNNING";
    std::string world_time = "2025-01-01T00:00:00Z";
};

struct ApiStatus {
    std::string engine_status = "RUNNING";
    std::string world_time = "2025-01-01T00:00:00Z";
    std::size_t entity_count = 0;
    std::size_t event_count = 0;
    std::size_t observation_count = 0;
    std::size_t relationship_count = 0;
    std::size_t source_count = 0;
    std::size_t knowledge_graph_size = 0;
};

struct GraphNode {
    std::string id;
    std::string label;
    std::string type;
    std::string summary;
};

struct GraphEdge {
    std::string id;
    std::string source;
    std::string target;
    std::string type;
};

struct GraphData {
    std::vector<GraphNode> nodes;
    std::vector<GraphEdge> edges;
};

struct SearchResult {
    std::string type;
    std::string id;
    std::string label;
    std::string detail;
};

struct ProvenanceEntry {
    std::string type;
    std::string id;
    std::string label;
    std::string detail;
};

struct ProvenanceView {
    std::vector<ProvenanceEntry> chain;
};

struct WorldStateResponse {
    std::string timestamp;
    std::vector<synthia::knowledge::Entity> entities;
    std::vector<synthia::knowledge::Event> events;
    std::vector<synthia::knowledge::Relationship> relationships;
    std::vector<synthia::knowledge::Observation> observations;
    std::vector<synthia::knowledge::Source> sources;
};

DashboardWorld create_demo_world();

ApiStatus make_status(const DashboardWorld& world);
std::vector<synthia::knowledge::Entity> list_entities(const DashboardWorld& world);
std::vector<synthia::knowledge::Event> list_events(const DashboardWorld& world);
std::vector<synthia::knowledge::Relationship> list_relationships(const DashboardWorld& world);
std::vector<synthia::knowledge::Observation> list_observations(const DashboardWorld& world);
std::vector<synthia::knowledge::Source> list_sources(const DashboardWorld& world);

synthia::knowledge::Entity get_entity(const DashboardWorld& world, const std::string& id);
synthia::knowledge::Event get_event(const DashboardWorld& world, const std::string& id);
synthia::knowledge::Observation get_observation(const DashboardWorld& world, const std::string& id);
synthia::knowledge::Source get_source(const DashboardWorld& world, const std::string& id);

GraphData make_graph(const DashboardWorld& world);
std::vector<std::pair<std::string, std::string>> timeline(const DashboardWorld& world);
WorldStateResponse make_world_state(const DashboardWorld& world, synthia::knowledge::Timestamp timestamp = 0);
std::vector<SearchResult> search(const DashboardWorld& world, const std::string& query);
ProvenanceView make_provenance(const DashboardWorld& world, const std::string& item_id);

std::string to_json(const ApiStatus& status);
std::string to_json(const synthia::knowledge::Entity& entity);
std::string to_json(const synthia::knowledge::Event& event);
std::string to_json(const synthia::knowledge::Relationship& relationship);
std::string to_json(const synthia::knowledge::Observation& observation);
std::string to_json(const synthia::knowledge::Source& source);
std::string to_json(const GraphData& graph);
std::string to_json(const WorldStateResponse& state);
std::string to_json(const std::vector<SearchResult>& results);
std::string to_json(const ProvenanceView& provenance);

}  // namespace api
}  // namespace synthia
