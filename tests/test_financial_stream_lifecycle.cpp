#include "synthia/finance/FinancialDataFabric.hpp"
#include "synthia/finance/FinancialStream.hpp"

#include <cassert>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace synthia::finance;

namespace {
struct MockState {
    std::vector<std::string> endpoints;
    std::vector<std::vector<std::string>> sent;
    std::vector<std::deque<std::string>> messages;
    std::vector<std::deque<std::string>> scripted_messages;
    std::vector<bool> connect_ok;
    std::vector<std::string> errors;
    std::size_t closes = 0;
    std::size_t active = 0;
    bool throw_receive = false;
};

class MockTransport final : public WebSocketTransport {
public:
    explicit MockTransport(std::shared_ptr<MockState> state) : state_(std::move(state)) {}
    bool connect(const std::string& endpoint, std::chrono::milliseconds, std::string& error) override {
        session_ = state_->endpoints.size();
        state_->endpoints.push_back(endpoint);
        state_->sent.emplace_back();
        state_->messages.push_back(session_ < state_->scripted_messages.size() ? state_->scripted_messages[session_] : std::deque<std::string>{});
        const bool success = session_ >= state_->connect_ok.size() || state_->connect_ok[session_];
        if (!success) error = state_->errors.empty() ? "mock connect failure" : state_->errors[session_];
        return success;
    }
    bool send_text(const std::string& message, std::string& error) override {
        if (session_ >= state_->sent.size()) { error = "not connected"; return false; }
        state_->sent[session_].push_back(message);
        return true;
    }
    WebSocketReceiveResult receive_text(std::chrono::milliseconds, std::string& message, std::string&) override {
        if (state_->throw_receive) throw std::runtime_error("mock receive exception");
        auto& queued = state_->messages[session_];
        if (queued.empty()) return WebSocketReceiveResult::Disconnected;
        message = queued.front();
        queued.pop_front();
        return WebSocketReceiveResult::Message;
    }
    void close() noexcept override { ++state_->closes; }
private:
    std::shared_ptr<MockState> state_;
    std::size_t session_ = 0;
};

FinnhubStreamConfig config() {
    FinnhubStreamConfig value;
    value.api_key = "test-token";
    value.symbols = {"AAPL", "MSFT"};
    value.maximum_reconnect_attempts = 0;
    value.initial_reconnect_delay = std::chrono::milliseconds(0);
    value.maximum_reconnect_delay = std::chrono::milliseconds(0);
    value.receive_timeout = std::chrono::milliseconds(1);
    return value;
}

const std::string trade = R"json({"type":"trade","data":[{"s":"AAPL","p":187.25,"t":1700000000123,"v":4.5} ]})json";

void setup_and_normalize() {
    auto mock = std::make_shared<MockState>();
    mock->scripted_messages = {{trade}};
    auto cfg = config();
    FinnhubWebSocketSource source(cfg, [mock] { return std::make_unique<MockTransport>(mock); });
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    std::atomic_bool stop{false};
    const auto success = source.run([&](const FinancialEvent& event) {
        assert(fabric.ingest_event(event).accepted == 1);
        stop.store(true);
    }, stop);
    assert(success);
    assert(source.status() == StreamConnectionStatus::Stopping);
    assert(mock->endpoints.size() == 1);
    assert(mock->endpoints.front().find("token=test-token") != std::string::npos);
    assert(mock->sent.front().size() == 2);
    assert(mock->sent.front()[0].find("AAPL") != std::string::npos);
    assert(mock->sent.front()[1].find("MSFT") != std::string::npos);
    assert(source.report().events == 1);
    assert(fabric.current_market_state().at("AAPL").last_price == 187.25);
    assert(world.observations().size() == 1);
    assert(world.observations().begin()->second.source_id == "finnhub_websocket");
}

void malformed_message() {
    auto mock = std::make_shared<MockState>();
    const auto overflowing = std::string(R"json({"type":"trade","data":[{"s":"AAPL","p":)json") +
        std::string(400, '9') + R"json(,"t":1700000000124,"v":1}]})json";
    mock->scripted_messages = {{R"json({"type":"trade","data":[{"s":"AAPL","p":"bad"}]})json", overflowing, trade}};
    FinnhubWebSocketSource source(config(), [mock] { return std::make_unique<MockTransport>(mock); });
    std::atomic_bool stop{false};
    source.run([&](const FinancialEvent&) { stop.store(true); }, stop);
    assert(source.report().malformed_messages == 2);
    assert(source.report().events == 1);
}

void duplicate_event() {
    auto mock = std::make_shared<MockState>();
    mock->scripted_messages = {{trade, trade}};
    FinnhubWebSocketSource source(config(), [mock] { return std::make_unique<MockTransport>(mock); });
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    std::atomic_bool stop{false};
    std::size_t received = 0;
    source.run([&](const FinancialEvent& event) {
        fabric.ingest_event(event);
        if (++received == 2) stop.store(true);
    }, stop);
    assert(source.report().events == 2);
    assert(fabric.records().size() == 1);
    assert(fabric.current_market_state().at("AAPL").realized_volume == 4.5);
}

void reconnect_and_shutdown() {
    auto mock = std::make_shared<MockState>();
    mock->scripted_messages = {{}, {trade}};
    FinnhubWebSocketSource source(config(), [mock] { return std::make_unique<MockTransport>(mock); });
    std::atomic_bool stop{false};
    const auto success = source.run([&](const FinancialEvent&) { stop.store(true); }, stop);
    assert(success);
    assert(mock->endpoints.size() == 2);
    assert(source.report().reconnects == 1);
    assert(source.status() == StreamConnectionStatus::Stopping);
}

void provider_failure() {
    auto mock = std::make_shared<MockState>();
    mock->connect_ok = {false, false};
    mock->errors = {"mock authentication/connect failure", "mock failure"};
    auto cfg = config();
    cfg.maximum_reconnect_attempts = 1;
    FinnhubWebSocketSource source(cfg, [mock] { return std::make_unique<MockTransport>(mock); });
    std::atomic_bool stop{false};
    assert(!source.run([](const FinancialEvent&) {}, stop));
    assert(mock->endpoints.size() == 2);
    assert(source.status() == StreamConnectionStatus::Disconnected);
    assert(!source.error().empty());

    auto throwing_mock = std::make_shared<MockState>();
    throwing_mock->throw_receive = true;
    auto throwing_config = config();
    throwing_config.maximum_reconnect_attempts = 1;
    FinnhubWebSocketSource throwing_source(throwing_config, [throwing_mock] { return std::make_unique<MockTransport>(throwing_mock); });
    assert(!throwing_source.run([](const FinancialEvent&) {}, stop));
    assert(throwing_source.status() == StreamConnectionStatus::Disconnected);
}

void auth_failure_and_missing_credentials() {
    auto mock = std::make_shared<MockState>();
    mock->scripted_messages = {{R"json({"type":"error","msg":"Invalid API key"})json"}};
    FinnhubWebSocketSource source(config(), [mock] { return std::make_unique<MockTransport>(mock); });
    std::atomic_bool stop{false};
    assert(!source.run([](const FinancialEvent&) {}, stop));
    assert(source.error().find("authentication error") != std::string::npos);
    assert(mock->endpoints.size() == 1);

    auto cfg = config();
    cfg.api_key.clear();
    FinnhubWebSocketSource no_credentials(cfg, [] { return std::make_unique<MockTransport>(std::make_shared<MockState>()); });
    assert(!no_credentials.run([](const FinancialEvent&) {}, stop));
    assert(no_credentials.status() == StreamConnectionStatus::Disconnected);
}
}

int main(int argc, char** argv) {
    assert(argc == 2);
    const std::string test = argv[1];
    if (test == "setup") setup_and_normalize();
    else if (test == "malformed") malformed_message();
    else if (test == "duplicate") duplicate_event();
    else if (test == "reconnect") reconnect_and_shutdown();
    else if (test == "failure") provider_failure();
    else if (test == "authentication") auth_failure_and_missing_credentials();
    else assert(false && "unknown test case");
    return 0;
}
