#include "synthia/finance/FinancialDataFabric.hpp"
#include "synthia/finance/FinancialIngestionSession.hpp"
#include "synthia/finance/FinancialStream.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace {
std::atomic_bool stop_requested{false};
static_assert(std::atomic_bool::is_always_lock_free, "signal stop flag must be lock-free");
void stop_test(int) { stop_requested.store(true); }

std::string environment(const char* name, const std::string& fallback = {}) {
    const char* value = std::getenv(name);
    return value == nullptr ? fallback : value;
}

std::vector<std::string> symbols(const std::string& value) {
    std::vector<std::string> result;
    std::stringstream input(value);
    std::string symbol;
    while (std::getline(input, symbol, ',')) {
        const auto first = symbol.find_first_not_of(" \t");
        const auto last = symbol.find_last_not_of(" \t");
        if (first != std::string::npos) result.push_back(symbol.substr(first, last - first + 1));
    }
    return result;
}
}

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "usage: synthia_finance_live_test [recording.json]\n";
        return 2;
    }
    synthia::finance::FinnhubStreamConfig config;
    config.api_key = environment("SYNTHIA_FINANCE_API_KEY");
    config.endpoint = environment("SYNTHIA_FINANCE_ENDPOINT", "wss://ws.finnhub.io");
    config.symbols = symbols(environment("SYNTHIA_FINANCE_SYMBOLS", "AAPL"));
    if (config.api_key.empty()) {
        std::cerr << "Set SYNTHIA_FINANCE_API_KEY in the environment before running this manual live integration test.\n";
        return 2;
    }

    synthia::knowledge::WorldState world;
    synthia::finance::FinancialDataFabric fabric(world);
    synthia::finance::FinnhubWebSocketSource source(std::move(config));
    std::signal(SIGINT, stop_test);
    std::signal(SIGTERM, stop_test);
    std::thread timeout([&] {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
        while (!stop_requested.load() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        stop_requested.store(true);
    });
    const auto connected = source.run([&](const synthia::finance::FinancialEvent& event) {
        const auto report = fabric.ingest_event(event);
        if (report.accepted > 0) {
            const auto& state = fabric.current_market_state().at(event.instrument.symbol);
            std::cout << "provider=" << event.source_id << " symbol=" << event.instrument.symbol
                      << " price=" << state.last_price << " event_timestamp=" << state.event_timestamp
                      << " world_entities=" << world.entities().size() << " provenance="
                      << world.observations().rbegin()->second.source_id << '\n';
            stop_requested.store(true);
        }
    }, stop_requested);
    stop_requested.store(true);
    timeout.join();

    bool world_updated = !fabric.current_market_state().empty() && !world.observations().empty();
    if (argc == 2 && world_updated) {
        const auto report = synthia::finance::IngestReport{source.report().events, fabric.observation_history().size(), 0, 0};
        synthia::finance::FinancialIngestionSession::capture("finnhub_live_smoke", fabric, report).save(argv[1]);
    }
    std::cout << "connection=" << synthia::finance::stream_connection_status_name(source.status())
              << " events=" << source.report().events << " malformed=" << source.report().malformed_messages
              << " world_state_updated=" << (world_updated ? "yes" : "no") << '\n';
    if (!source.error().empty()) std::cerr << source.error() << '\n';
    return connected && world_updated ? 0 : 1;
}
