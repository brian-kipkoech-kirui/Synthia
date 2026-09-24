#include "synthia/Agent.hpp"
#include "synthia/World.hpp"
#include "synthia/knowledge/WorldState.hpp"

#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

namespace {
using synthia::Action;
using synthia::ActionType;
using synthia::Agent;
using synthia::Observation;
using synthia::World;
using synthia::WorldConfig;
using KnowledgeEntity = synthia::knowledge::Entity;
using KnowledgeEvent = synthia::knowledge::Event;
using KnowledgeObservation = synthia::knowledge::Observation;
using KnowledgeRelationship = synthia::knowledge::Relationship;
using KnowledgeSource = synthia::knowledge::Source;
using KnowledgeWorldState = synthia::knowledge::WorldState;

std::string quote(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (const char character : value) {
        switch (character) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << character; break;
        }
    }
    out << '"';
    return out.str();
}

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::map<std::string, std::string> form_values(const std::string& body) {
    std::map<std::string, std::string> values;
    std::stringstream input(body);
    std::string pair;
    while (std::getline(input, pair, '&')) {
        const auto split = pair.find('=');
        values[pair.substr(0, split)] = split == std::string::npos ? "" : pair.substr(split + 1);
    }
    return values;
}

class DashboardAgent final : public Agent {
public:
    DashboardAgent() : Agent("agent_0", "navigate") {}

    Observation observe(const World& world) override {
        Observation observation;
        observation.agent_id = id();
        observation.goal = goal();
        observation.agent_position = world.agent_position(id());
        observation.target_position = world.agent_target_position(id());
        observation.distance_to_target = world.distance(observation.agent_position, observation.target_position);
        observation.energy = world.agent_energy(id());
        observation.simulation_time = world.simulation_time();
        observation.environment = world.environment();
        observation.terrain = world.terrain_at(observation.agent_position);
        return observation;
    }

    Action decide(const Observation& observation) override {
        if (observation.agent_position.x < observation.target_position.x) return {ActionType::MoveRight, "", 1.0};
        if (observation.agent_position.z < observation.target_position.z) return {ActionType::MoveUp, "", 1.0};
        return {ActionType::Wait, "", 0.0};
    }
};

class DashboardRuntime {
public:
    DashboardRuntime() { load("multi_agent_nav", 42); }

    void load(const std::string& scenario, unsigned int seed) {
        scenario_ = scenario;
        WorldConfig config{scenario, seed, 20, 20, 100, "urban"};
        config.start_position = {1.0, 0.0, 1.0};
        config.target_position = {12.0, 0.0, 12.0};
        world_ = std::make_unique<World>(config);
        world_->add_agent("agent_0", config.start_position, config.target_position);
        world_->add_resource("resource_0", {8.0, 0.0, 8.0}, "supply");
        world_->add_obstacle("obstacle_0", {5.0, 0.0, 5.0});
        agent_ = std::make_unique<DashboardAgent>();
        knowledge_ = KnowledgeWorldState{};
        ingest_state();
        running_ = false;
    }

    void start() { running_ = true; }
    void pause() { running_ = false; }
    void step() {
        const auto observation = agent_->observe(*world_);
        const auto action = agent_->decide(observation);
        const auto simulation = world_->step(agent_->id(), action);
        agent_->on_step_result(action, simulation, *world_);
        ingest_state();
    }
    void reset() { load(scenario_, world_->seed()); }

    std::string state_json() const {
        std::ostringstream out;
        out << "{\"scenario\":" << quote(scenario_) << ",\"seed\":" << world_->seed()
            << ",\"tick\":" << world_->step_count() << ",\"time\":" << world_->time()
            << ",\"running\":" << (running_ ? "true" : "false")
            << ",\"world\":{\"width\":" << world_->width() << ",\"height\":" << world_->height() << "}"
            << ",\"environment\":{\"temperature\":" << world_->environment().temperature
            << ",\"weather\":" << quote(world_->environment().weather)
            << ",\"hazard_level\":" << world_->environment().hazard_level << "}"
            << ",\"agents\":" << entities_json(true) << ",\"entities\":" << entities_json(false)
            << ",\"events\":" << events_json() << "}";
        return out.str();
    }

    std::string agents_json() const { return entities_json(true); }
    std::string entities_json(bool agents_only) const {
        std::ostringstream out;
        out << '[';
        bool first = true;
        for (const auto& entity : world_->entities()) {
            if (agents_only != (entity.type == "agent")) continue;
            if (!first) out << ',';
            first = false;
            out << entity_json(entity);
        }
        out << ']';
        return out.str();
    }

    std::string events_json() const {
        std::ostringstream out;
        out << '[';
        bool first = true;
        for (const auto& event : world_->events()) {
            if (!first) out << ',';
            first = false;
            out << event_json(event);
        }
        out << ']';
        return out.str();
    }

    std::string knowledge_json() const {
        std::ostringstream out;
        out << "{\"entities\":[";
        bool first = true;
        for (const auto& item : knowledge_.entities()) { if (!first) out << ','; first = false; out << knowledge_entity_json(item.second); }
        out << "],\"events\":[";
        first = true;
        for (const auto& item : knowledge_.events()) { if (!first) out << ','; first = false; out << knowledge_event_json(item.second); }
        out << "],\"relationships\":[";
        first = true;
        for (const auto& item : knowledge_.relationships()) { if (!first) out << ','; first = false; out << relationship_json(item.second); }
        out << "],\"observations\":[";
        first = true;
        for (const auto& item : knowledge_.observations()) { if (!first) out << ','; first = false; out << knowledge_observation_json(item.second); }
        out << "],\"sources\":[";
        first = true;
        for (const auto& item : knowledge_.sources()) { if (!first) out << ','; first = false; out << source_json(item.second); }
        out << "]}";
        return out.str();
    }

private:
    static std::string entity_json(const synthia::Entity& entity) {
        std::ostringstream out;
        out << "{\"id\":" << quote(entity.id) << ",\"type\":" << quote(entity.type)
            << ",\"x\":" << entity.position.x << ",\"y\":" << entity.position.y << ",\"z\":" << entity.position.z
            << ",\"vx\":" << entity.velocity.x << ",\"vy\":" << entity.velocity.y << ",\"vz\":" << entity.velocity.z
            << ",\"energy\":" << entity.energy << ",\"active\":" << (entity.active ? "true" : "false")
            << ",\"metadata\":" << quote(entity.metadata) << "}";
        return out.str();
    }

    static std::string event_json(const synthia::WorldEvent& event) {
        std::ostringstream out;
        out << "{\"id\":" << quote(event.id) << ",\"type\":" << quote(event.type)
            << ",\"actor_id\":" << quote(event.actor_id) << ",\"x\":" << event.position.x
            << ",\"y\":" << event.position.y << ",\"z\":" << event.position.z
            << ",\"message\":" << quote(event.message) << "}";
        return out.str();
    }
    static std::string knowledge_entity_json(const KnowledgeEntity& entity) {
        return "{\"id\":" + quote(entity.id) + ",\"name\":" + quote(entity.canonical_name) + ",\"type\":" + quote(entity.entity_type) + ",\"description\":" + quote(entity.description) + "}";
    }
    static std::string knowledge_event_json(const KnowledgeEvent& event) {
        return "{\"id\":" + quote(event.id) + ",\"timestamp\":" + std::to_string(event.timestamp) + ",\"type\":" + quote(event.event_type) + ",\"description\":" + quote(event.description) + "}";
    }
    static std::string relationship_json(const KnowledgeRelationship& relationship) {
        return "{\"id\":" + quote(relationship.id) + ",\"source\":" + quote(relationship.source_entity_id) + ",\"target\":" + quote(relationship.target_entity_id) + ",\"type\":" + quote(relationship.relationship_type) + ",\"provenance\":" + quote(relationship.support_observation_id) + "}";
    }
    static std::string knowledge_observation_json(const KnowledgeObservation& observation) {
        return "{\"id\":" + quote(observation.id) + ",\"timestamp\":" + std::to_string(observation.timestamp) + ",\"source_id\":" + quote(observation.source_id) + ",\"content\":" + quote(observation.content) + ",\"confidence\":" + std::to_string(observation.confidence) + "}";
    }
    static std::string source_json(const KnowledgeSource& source) {
        return "{\"id\":" + quote(source.id) + ",\"name\":" + quote(source.name) + ",\"type\":" + quote(source.source_type) + "}";
    }

    void ingest_state() {
        const KnowledgeSource source{"source_simulation", "SYNTHIA simulation", "synthia://simulation", "simulation"};
        knowledge_.add_source(source);
        const auto timestamp = static_cast<synthia::knowledge::Timestamp>(world_->simulation_time());
        for (const auto& entity : world_->entities()) {
            KnowledgeEntity knowledge_entity;
            knowledge_entity.id = entity.id;
            knowledge_entity.canonical_name = entity.id;
            knowledge_entity.entity_type = entity.type;
            knowledge_entity.description = entity.metadata;
            knowledge_.add_entity(knowledge_entity);
            KnowledgeObservation observation;
            observation.id = "obs_" + entity.id + "_" + std::to_string(timestamp);
            observation.timestamp = timestamp;
            observation.source_id = source.id;
            observation.data_type = "simulation_state";
            observation.confidence = 1.0;
            observation.content = entity.id + " observed at simulation tick " + std::to_string(world_->step_count());
            observation.related_entity_ids = {entity.id};
            knowledge_.apply(observation);
        }
        for (const auto& event : world_->events()) {
            KnowledgeEvent knowledge_event;
            knowledge_event.id = "event_" + event.id + "_" + event.type;
            knowledge_event.timestamp = timestamp;
            knowledge_event.event_type = event.type;
            knowledge_event.description = event.message;
            knowledge_event.entity_ids = {event.actor_id};
            knowledge_.add_event(knowledge_event);
        }
    }

    std::unique_ptr<World> world_;
    std::unique_ptr<DashboardAgent> agent_;
    KnowledgeWorldState knowledge_;
    std::string scenario_;
    bool running_ = false;
};

std::string quote_error(const std::string& message) { return "{\"error\":" + quote(message) + "}"; }
std::string ok_response(const std::string& type, const std::string& body) {
    return "HTTP/1.1 200 OK\r\nContent-Type: " + type + "\r\nContent-Length: " + std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
}

void handle_client(int socket, DashboardRuntime& runtime, std::mutex& mutex) {
    char buffer[16384];
    const auto bytes = recv(socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) { close(socket); return; }
    buffer[bytes] = '\0';
    const std::string request(buffer, static_cast<std::size_t>(bytes));
    std::stringstream line(request.substr(0, request.find("\r\n")));
    std::string method, target, version;
    line >> method >> target >> version;
    const auto path_end = target.find('?');
    const auto path = target.substr(0, path_end);
    const auto body_start = request.find("\r\n\r\n");
    const std::string body = body_start == std::string::npos ? "" : request.substr(body_start + 4);
    std::string result;
    std::string content_type = "application/json";
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (method == "GET" && path == "/api/state") result = runtime.state_json();
        else if (method == "GET" && path == "/api/agents") result = runtime.agents_json();
        else if (method == "GET" && path == "/api/entities") result = runtime.entities_json(false);
        else if (method == "GET" && path == "/api/events") result = runtime.events_json();
        else if (method == "GET" && path == "/api/knowledge") result = runtime.knowledge_json();
        else if (method == "POST" && path == "/api/simulation/start") { runtime.start(); result = runtime.state_json(); }
        else if (method == "POST" && path == "/api/simulation/pause") { runtime.pause(); result = runtime.state_json(); }
        else if (method == "POST" && path == "/api/simulation/step") { runtime.step(); result = runtime.state_json(); }
        else if (method == "POST" && path == "/api/simulation/reset") { runtime.reset(); result = runtime.state_json(); }
        else if (method == "POST" && path == "/api/scenario/load") {
            const auto values = form_values(body);
            runtime.load(values.count("scenario") ? values.at("scenario") : "multi_agent_nav", values.count("seed") ? static_cast<unsigned int>(std::stoul(values.at("seed"))) : 42);
            result = runtime.state_json();
        } else if (path == "/" || path == "/index.html" || path == "/app.js" || path == "/styles.css") {
            content_type = path == "/app.js" ? "application/javascript" : (path == "/styles.css" ? "text/css" : "text/html");
            result = read_file(path == "/" ? "web/index.html" : "web" + path);
        } else result = quote_error("Not found");
    }
    send(socket, ok_response(content_type, result).c_str(), ok_response(content_type, result).size(), 0);
    close(socket);
}

}  // namespace

int main() {
    DashboardRuntime runtime;
    std::mutex mutex;
    const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) return 1;
    int reuse = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(server_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 || listen(server_socket, 16) < 0) return 1;
    std::cout << "SYNTHIA Web Observatory listening on http://localhost:8080\n";
    while (true) {
        sockaddr_in client_address{};
        socklen_t length = sizeof(client_address);
        const int client = accept(server_socket, reinterpret_cast<sockaddr*>(&client_address), &length);
        if (client >= 0) std::thread(handle_client, client, std::ref(runtime), std::ref(mutex)).detach();
    }
}
