#include "synthia/api/DashboardApi.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>

namespace synthia {
namespace api {
namespace {

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string escape_json(const std::string& value) {
    std::ostringstream out;
    for (char ch : value) {
        switch (ch) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20u) {
                    out << "\\u00" << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(ch)) << std::nouppercase;
                } else {
                    out << ch;
                }
                break;
        }
    }
    return out.str();
}

std::string json_array_strings(const std::vector<std::string>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += '"' + escape_json(values[i]) + '"';
    }
    out += "]";
    return out;
}

std::string json_array_entities(const std::vector<synthia::knowledge::Entity>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += to_json(values[i]);
    }
    out += "]";
    return out;
}

std::string json_array_events(const std::vector<synthia::knowledge::Event>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += to_json(values[i]);
    }
    out += "]";
    return out;
}

std::string json_array_relationships(const std::vector<synthia::knowledge::Relationship>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += to_json(values[i]);
    }
    out += "]";
    return out;
}

std::string json_array_observations(const std::vector<synthia::knowledge::Observation>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += to_json(values[i]);
    }
    out += "]";
    return out;
}

std::string json_array_sources(const std::vector<synthia::knowledge::Source>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += to_json(values[i]);
    }
    out += "]";
    return out;
}

std::string json_array_graph_nodes(const std::vector<GraphNode>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += "{\"id\":\"" + escape_json(values[i].id) + "\",\"label\":\"" + escape_json(values[i].label) + "\",\"type\":\"" + escape_json(values[i].type) + "\",\"summary\":\"" + escape_json(values[i].summary) + "\"}";
    }
    out += "]";
    return out;
}

std::string json_array_graph_edges(const std::vector<GraphEdge>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += "{\"id\":\"" + escape_json(values[i].id) + "\",\"source\":\"" + escape_json(values[i].source) + "\",\"target\":\"" + escape_json(values[i].target) + "\",\"type\":\"" + escape_json(values[i].type) + "\"}";
    }
    out += "]";
    return out;
}

std::string json_array_provenance(const std::vector<ProvenanceEntry>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += "{\"type\":\"" + escape_json(values[i].type) + "\",\"id\":\"" + escape_json(values[i].id) + "\",\"label\":\"" + escape_json(values[i].label) + "\",\"detail\":\"" + escape_json(values[i].detail) + "\"}";
    }
    out += "]";
    return out;
}

std::string json_array_search_results(const std::vector<SearchResult>& values) {
    std::string out = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out += ",";
        out += "{\"type\":\"" + escape_json(values[i].type) + "\",\"id\":\"" + escape_json(values[i].id) + "\",\"label\":\"" + escape_json(values[i].label) + "\",\"detail\":\"" + escape_json(values[i].detail) + "\"}";
    }
    out += "]";
    return out;
}

std::string state_timestamp_string(synthia::knowledge::Timestamp ts) {
    if (ts == 0) {
        return "2025-01-01T00:00:00Z";
    }
    std::ostringstream out;
    out << ts;
    return out.str();
}

bool matches_query(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) {
        return true;
    }
    const auto lower_haystack = to_lower(haystack);
    const auto lower_needle = to_lower(needle);
    return lower_haystack.find(lower_needle) != std::string::npos;
}

}  // namespace

DashboardWorld create_demo_world() {
    DashboardWorld world;
    auto& state = world.state;

    synthia::knowledge::Source ledger;
    ledger.id = "source_ledger";
    ledger.name = "Daily Ledger";
    ledger.uri = "https://example.org/ledger";
    ledger.source_type = "news";
    state.add_source(ledger);

    synthia::knowledge::Source field_office;
    field_office.id = "source_field_office";
    field_office.name = "Field Intelligence Office";
    field_office.uri = "https://example.org/field";
    field_office.source_type = "field_report";
    state.add_source(field_office);

    synthia::knowledge::Entity alpha;
    alpha.id = "entity_alpha";
    alpha.canonical_name = "Company Alpha";
    alpha.entity_type = "company";
    alpha.description = "A logistics and shipping company";
    state.add_entity(alpha);

    synthia::knowledge::Entity beta;
    beta.id = "entity_beta";
    beta.canonical_name = "Company Beta";
    beta.entity_type = "company";
    beta.description = "A retail distribution company";
    state.add_entity(beta);

    synthia::knowledge::Entity country_x;
    country_x.id = "entity_country_x";
    country_x.canonical_name = "Country X";
    country_x.entity_type = "country";
    country_x.description = "A regional market";
    state.add_entity(country_x);

    synthia::knowledge::Observation obs_alpha;
    obs_alpha.id = "obs_alpha_foundation";
    obs_alpha.timestamp = 20250105;
    obs_alpha.source_id = "source_ledger";
    obs_alpha.content = "Company Alpha was founded in Country X with a regional logistics strategy.";
    obs_alpha.data_type = "news";
    obs_alpha.confidence = 0.97;
    obs_alpha.related_entity_ids = {"entity_alpha", "entity_country_x"};
    state.apply(obs_alpha);

    synthia::knowledge::Observation obs_beta;
    obs_beta.id = "obs_beta_launch";
    obs_beta.timestamp = 20250206;
    obs_beta.source_id = "source_field_office";
    obs_beta.content = "Company Beta opened distribution in the western region of Country X.";
    obs_beta.data_type = "field_report";
    obs_beta.confidence = 0.91;
    obs_beta.related_entity_ids = {"entity_beta", "entity_country_x"};
    state.apply(obs_beta);

    synthia::knowledge::Observation obs_partnership;
    obs_partnership.id = "obs_partnership";
    obs_partnership.timestamp = 20250301;
    obs_partnership.source_id = "source_ledger";
    obs_partnership.content = "Company Alpha and Company Beta signed a logistics partnership.";
    obs_partnership.data_type = "news";
    obs_partnership.confidence = 0.94;
    obs_partnership.related_entity_ids = {"entity_alpha", "entity_beta"};
    state.apply(obs_partnership);

    synthia::knowledge::Event event_foundation;
    event_foundation.id = "event_alpha_foundation";
    event_foundation.timestamp = 20250105;
    event_foundation.event_type = "founding";
    event_foundation.description = "Company Alpha founded in Country X";
    event_foundation.entity_ids = {"entity_alpha", "entity_country_x"};
    state.add_event(event_foundation);

    synthia::knowledge::Event event_market;
    event_market.id = "event_market_change";
    event_market.timestamp = 20250215;
    event_market.event_type = "market";
    event_market.description = "Regional demand increased for logistics capacity";
    event_market.entity_ids = {"entity_alpha", "entity_beta"};
    state.add_event(event_market);

    synthia::knowledge::Event event_partnership;
    event_partnership.id = "event_alpha_beta_partnership";
    event_partnership.timestamp = 20250301;
    event_partnership.event_type = "partnership";
    event_partnership.description = "Company Alpha partnered with Company Beta";
    event_partnership.entity_ids = {"entity_alpha", "entity_beta"};
    state.add_event(event_partnership);

    synthia::knowledge::Relationship alpha_operates;
    alpha_operates.id = "rel_alpha_country_x";
    alpha_operates.source_entity_id = "entity_alpha";
    alpha_operates.target_entity_id = "entity_country_x";
    alpha_operates.relationship_type = "operates_in";
    alpha_operates.valid_from = 20250105;
    alpha_operates.valid_to = synthia::knowledge::kNoUpperBound;
    alpha_operates.description = "Company Alpha operates in Country X";
    alpha_operates.support_observation_id = "obs_alpha_foundation";
    state.add_relationship(alpha_operates);

    synthia::knowledge::Relationship beta_operates;
    beta_operates.id = "rel_beta_country_x";
    beta_operates.source_entity_id = "entity_beta";
    beta_operates.target_entity_id = "entity_country_x";
    beta_operates.relationship_type = "operates_in";
    beta_operates.valid_from = 20250206;
    beta_operates.valid_to = synthia::knowledge::kNoUpperBound;
    beta_operates.description = "Company Beta operates in Country X";
    beta_operates.support_observation_id = "obs_beta_launch";
    state.add_relationship(beta_operates);

    synthia::knowledge::Relationship alpha_beta;
    alpha_beta.id = "rel_alpha_beta";
    alpha_beta.source_entity_id = "entity_alpha";
    alpha_beta.target_entity_id = "entity_beta";
    alpha_beta.relationship_type = "partnered_with";
    alpha_beta.valid_from = 20250301;
    alpha_beta.valid_to = synthia::knowledge::kNoUpperBound;
    alpha_beta.description = "Company Alpha partnered with Company Beta";
    alpha_beta.support_observation_id = "obs_partnership";
    state.add_relationship(alpha_beta);

    world.world_time = "2025-03-15T00:00:00Z";
    return world;
}

ApiStatus make_status(const DashboardWorld& world) {
    const auto& state = world.state;
    ApiStatus status;
    status.engine_status = world.engine_status;
    status.world_time = world.world_time;
    status.entity_count = state.entities().size();
    status.event_count = state.events().size();
    status.observation_count = state.observations().size();
    status.relationship_count = state.relationships().size();
    status.source_count = state.sources().size();
    status.knowledge_graph_size = state.graph().entities().size() + state.graph().events().size() + state.graph().relationships().size();
    return status;
}

std::vector<synthia::knowledge::Entity> list_entities(const DashboardWorld& world) {
    std::vector<synthia::knowledge::Entity> values;
    for (const auto& [id, entity] : world.state.entities()) {
        values.push_back(entity);
    }
    return values;
}

std::vector<synthia::knowledge::Event> list_events(const DashboardWorld& world) {
    std::vector<synthia::knowledge::Event> values;
    for (const auto& [id, event] : world.state.events()) {
        values.push_back(event);
    }
    return values;
}

std::vector<synthia::knowledge::Relationship> list_relationships(const DashboardWorld& world) {
    std::vector<synthia::knowledge::Relationship> values;
    for (const auto& [id, relationship] : world.state.relationships()) {
        values.push_back(relationship);
    }
    return values;
}

std::vector<synthia::knowledge::Observation> list_observations(const DashboardWorld& world) {
    std::vector<synthia::knowledge::Observation> values;
    for (const auto& [id, observation] : world.state.observations()) {
        values.push_back(observation);
    }
    return values;
}

std::vector<synthia::knowledge::Source> list_sources(const DashboardWorld& world) {
    std::vector<synthia::knowledge::Source> values;
    for (const auto& [id, source] : world.state.sources()) {
        values.push_back(source);
    }
    return values;
}

synthia::knowledge::Entity get_entity(const DashboardWorld& world, const std::string& id) {
    const auto* entity = world.state.get_entity(id);
    synthia::knowledge::Entity empty{};
    if (entity == nullptr) {
        return empty;
    }
    return *entity;
}

synthia::knowledge::Event get_event(const DashboardWorld& world, const std::string& id) {
    const auto* event = world.state.index().get_event_by_id(id);
    synthia::knowledge::Event empty{};
    if (event == nullptr) {
        return empty;
    }
    return *event;
}

synthia::knowledge::Observation get_observation(const DashboardWorld& world, const std::string& id) {
    const auto* observation = world.state.index().get_observation_by_id(id);
    synthia::knowledge::Observation empty{};
    if (observation == nullptr) {
        return empty;
    }
    return *observation;
}

synthia::knowledge::Source get_source(const DashboardWorld& world, const std::string& id) {
    const auto* source = world.state.index().get_source_by_id(id);
    synthia::knowledge::Source empty{};
    if (source == nullptr) {
        return empty;
    }
    return *source;
}

GraphData make_graph(const DashboardWorld& world) {
    GraphData graph;
    for (const auto& [id, entity] : world.state.entities()) {
        GraphNode node;
        node.id = entity.id;
        node.label = entity.canonical_name;
        node.type = entity.entity_type;
        node.summary = entity.description;
        graph.nodes.push_back(node);
    }
    for (const auto& [id, relationship] : world.state.relationships()) {
        GraphEdge edge;
        edge.id = relationship.id;
        edge.source = relationship.source_entity_id;
        edge.target = relationship.target_entity_id;
        edge.type = relationship.relationship_type;
        graph.edges.push_back(edge);
    }
    return graph;
}

std::vector<std::pair<std::string, std::string>> timeline(const DashboardWorld& world) {
    std::vector<std::pair<std::string, std::string>> points;
    for (const auto& [id, event] : world.state.events()) {
        points.emplace_back(std::to_string(event.timestamp), event.description);
    }
    std::sort(points.begin(), points.end(), [](const auto& lhs, const auto& rhs) {
        return std::stoll(lhs.first) < std::stoll(rhs.first);
    });
    return points;
}

WorldStateResponse make_world_state(const DashboardWorld& world, synthia::knowledge::Timestamp timestamp) {
    WorldStateResponse state;
    const auto effective_timestamp = timestamp == 0 ? 20250315 : timestamp;
    state.timestamp = state_timestamp_string(effective_timestamp);
    for (const auto& [id, entity] : world.state.entities()) {
        state.entities.push_back(entity);
    }
    for (const auto& [id, event] : world.state.events()) {
        if (event.timestamp <= effective_timestamp) {
            state.events.push_back(event);
        }
    }
    for (const auto& [id, relationship] : world.state.relationships()) {
        if (relationship.valid_from <= effective_timestamp && (relationship.valid_to == synthia::knowledge::kNoUpperBound || relationship.valid_to >= effective_timestamp)) {
            state.relationships.push_back(relationship);
        }
    }
    for (const auto& [id, observation] : world.state.observations()) {
        if (observation.timestamp <= effective_timestamp) {
            state.observations.push_back(observation);
        }
    }
    for (const auto& [id, source] : world.state.sources()) {
        state.sources.push_back(source);
    }
    return state;
}

std::vector<SearchResult> search(const DashboardWorld& world, const std::string& query) {
    std::vector<SearchResult> results;
    if (query.empty()) {
        return results;
    }
    for (const auto& [id, entity] : world.state.entities()) {
        if (matches_query(entity.canonical_name, query) || matches_query(entity.description, query) || matches_query(entity.entity_type, query)) {
            SearchResult result; result.type = "entity"; result.id = entity.id; result.label = entity.canonical_name; result.detail = entity.entity_type;
            results.push_back(result);
        }
    }
    for (const auto& [id, event] : world.state.events()) {
        if (matches_query(event.description, query) || matches_query(event.event_type, query)) {
            SearchResult result; result.type = "event"; result.id = event.id; result.label = event.event_type; result.detail = event.description;
            results.push_back(result);
        }
    }
    for (const auto& [id, relationship] : world.state.relationships()) {
        if (matches_query(relationship.description, query) || matches_query(relationship.relationship_type, query)) {
            SearchResult result; result.type = "relationship"; result.id = relationship.id; result.label = relationship.relationship_type; result.detail = relationship.description;
            results.push_back(result);
        }
    }
    for (const auto& [id, observation] : world.state.observations()) {
        if (matches_query(observation.content, query) || matches_query(observation.data_type, query)) {
            SearchResult result; result.type = "observation"; result.id = observation.id; result.label = observation.data_type; result.detail = observation.content;
            results.push_back(result);
        }
    }
    for (const auto& [id, source] : world.state.sources()) {
        if (matches_query(source.name, query) || matches_query(source.source_type, query) || matches_query(source.uri, query)) {
            SearchResult result; result.type = "source"; result.id = source.id; result.label = source.name; result.detail = source.uri;
            results.push_back(result);
        }
    }
    return results;
}

ProvenanceView make_provenance(const DashboardWorld& world, const std::string& item_id) {
    ProvenanceView view;
    const auto* relationship = world.state.index().get_relationship_by_id(item_id);
    if (relationship != nullptr) {
        ProvenanceEntry root; root.type = "relationship"; root.id = relationship->id; root.label = relationship->relationship_type; root.detail = relationship->description; view.chain.push_back(root);
        ProvenanceEntry obs; obs.type = "observation"; obs.id = relationship->support_observation_id; obs.label = "Supporting observation"; obs.detail = world.state.index().get_observation_by_id(relationship->support_observation_id) != nullptr ? world.state.index().get_observation_by_id(relationship->support_observation_id)->content : ""; view.chain.push_back(obs);
        const auto* source = world.state.index().get_source_by_id(world.state.index().get_observation_by_id(relationship->support_observation_id)->source_id);
        if (source != nullptr) {
            ProvenanceEntry src; src.type = "source"; src.id = source->id; src.label = source->name; src.detail = source->uri; view.chain.push_back(src);
        }
        return view;
    }

    const auto* event = world.state.index().get_event_by_id(item_id);
    if (event != nullptr) {
        ProvenanceEntry entry; entry.type = "event"; entry.id = event->id; entry.label = event->event_type; entry.detail = event->description; view.chain.push_back(entry);
        for (const auto& entity_id : event->entity_ids) {
            ProvenanceEntry entity_entry; entity_entry.type = "entity"; entity_entry.id = entity_id; entity_entry.label = world.state.get_entity(entity_id) != nullptr ? world.state.get_entity(entity_id)->canonical_name : entity_id; entity_entry.detail = "Event participant"; view.chain.push_back(entity_entry);
        }
        return view;
    }

    const auto* observation = world.state.index().get_observation_by_id(item_id);
    if (observation != nullptr) {
        ProvenanceEntry entry; entry.type = "observation"; entry.id = observation->id; entry.label = observation->data_type; entry.detail = observation->content; view.chain.push_back(entry);
        const auto* source = world.state.index().get_source_by_id(observation->source_id);
        if (source != nullptr) {
            ProvenanceEntry src; src.type = "source"; src.id = source->id; src.label = source->name; src.detail = source->uri; view.chain.push_back(src);
        }
        return view;
    }

    const auto* entity = world.state.get_entity(item_id);
    if (entity != nullptr) {
        ProvenanceEntry entry; entry.type = "entity"; entry.id = entity->id; entry.label = entity->canonical_name; entry.detail = entity->description; view.chain.push_back(entry);
        for (const auto& relationship : world.state.get_relationships(item_id)) {
            ProvenanceEntry rel_entry; rel_entry.type = "relationship"; rel_entry.id = relationship.id; rel_entry.label = relationship.relationship_type; rel_entry.detail = relationship.description; view.chain.push_back(rel_entry);
        }
    }
    return view;
}

std::string to_json(const ApiStatus& status) {
    return "{\"engine_status\":\"" + escape_json(status.engine_status) + "\",\"world_time\":\"" + escape_json(status.world_time) + "\",\"entity_count\":" + std::to_string(status.entity_count) + ",\"event_count\":" + std::to_string(status.event_count) + ",\"observation_count\":" + std::to_string(status.observation_count) + ",\"relationship_count\":" + std::to_string(status.relationship_count) + ",\"source_count\":" + std::to_string(status.source_count) + ",\"knowledge_graph_size\":" + std::to_string(status.knowledge_graph_size) + "}";
}

std::string to_json(const synthia::knowledge::Entity& entity) {
    return "{\"id\":\"" + escape_json(entity.id) + "\",\"canonical_name\":\"" + escape_json(entity.canonical_name) + "\",\"entity_type\":\"" + escape_json(entity.entity_type) + "\",\"aliases\":" + json_array_strings(entity.aliases) + ",\"description\":\"" + escape_json(entity.description) + "\"}";
}

std::string to_json(const synthia::knowledge::Event& event) {
    return "{\"id\":\"" + escape_json(event.id) + "\",\"timestamp\":" + std::to_string(event.timestamp) + ",\"event_type\":\"" + escape_json(event.event_type) + "\",\"description\":\"" + escape_json(event.description) + "\",\"entity_ids\":" + json_array_strings(event.entity_ids) + "}";
}

std::string to_json(const synthia::knowledge::Relationship& relationship) {
    return "{\"id\":\"" + escape_json(relationship.id) + "\",\"source_entity_id\":\"" + escape_json(relationship.source_entity_id) + "\",\"target_entity_id\":\"" + escape_json(relationship.target_entity_id) + "\",\"relationship_type\":\"" + escape_json(relationship.relationship_type) + "\",\"valid_from\":" + std::to_string(relationship.valid_from) + ",\"valid_to\":" + std::to_string(relationship.valid_to) + ",\"description\":\"" + escape_json(relationship.description) + "\",\"support_observation_id\":\"" + escape_json(relationship.support_observation_id) + "\"}";
}

std::string to_json(const synthia::knowledge::Observation& observation) {
    return "{\"id\":\"" + escape_json(observation.id) + "\",\"timestamp\":" + std::to_string(observation.timestamp) + ",\"source_id\":\"" + escape_json(observation.source_id) + "\",\"content\":\"" + escape_json(observation.content) + "\",\"data_type\":\"" + escape_json(observation.data_type) + "\",\"confidence\":" + std::to_string(observation.confidence) + ",\"related_entity_ids\":" + json_array_strings(observation.related_entity_ids) + ",\"related_event_ids\":" + json_array_strings(observation.related_event_ids) + "}";
}

std::string to_json(const synthia::knowledge::Source& source) {
    return "{\"id\":\"" + escape_json(source.id) + "\",\"name\":\"" + escape_json(source.name) + "\",\"uri\":\"" + escape_json(source.uri) + "\",\"source_type\":\"" + escape_json(source.source_type) + "\"}";
}

std::string to_json(const GraphData& graph) {
    return "{\"nodes\":" + json_array_graph_nodes(graph.nodes) + ",\"edges\":" + json_array_graph_edges(graph.edges) + "}";
}

std::string to_json(const WorldStateResponse& state) {
    return "{\"timestamp\":\"" + escape_json(state.timestamp) + "\",\"entities\":" + json_array_entities(state.entities) + ",\"events\":" + json_array_events(state.events) + ",\"relationships\":" + json_array_relationships(state.relationships) + ",\"observations\":" + json_array_observations(state.observations) + ",\"sources\":" + json_array_sources(state.sources) + "}";
}

std::string to_json(const std::vector<SearchResult>& results) {
    return json_array_search_results(results);
}

std::string to_json(const ProvenanceView& provenance) {
    return "{\"chain\":" + json_array_provenance(provenance.chain) + "}";
}

}  // namespace api
}  // namespace synthia
