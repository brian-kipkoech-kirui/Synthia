#include "synthia/finance/FinancialIngestionSession.hpp"
#include "synthia/finance/FinancialStream.hpp"
#include "synthia/finance/FinancialSources.hpp"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {
std::atomic_bool stopped{false};
static_assert(std::atomic_bool::is_always_lock_free, "signal stop flag must be lock-free");
void stop_polling(int) { stopped.store(true); }

std::string environment(const char* name, const std::string& fallback = {}) {
    const char* value = std::getenv(name);
    return value == nullptr ? fallback : value;
}

std::vector<std::string> split_symbols(const std::string& symbols) {
    std::vector<std::string> result;
    std::stringstream input(symbols);
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
    stopped.store(false);
    std::string provider = environment("SYNTHIA_FINANCE_PROVIDER", "yahoo");
    std::string mode = environment("SYNTHIA_FINANCE_MODE", "snapshot");
    std::vector<std::string> symbols = split_symbols(environment("SYNTHIA_FINANCE_SYMBOLS"));
    std::string period = "1d";
    std::string interval = "1m";
    std::string output;
    std::string replay_path;
    int interval_seconds = 15;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--provider" && index + 1 < argc) provider = argv[++index];
        else if (argument == "--mode" && index + 1 < argc) mode = argv[++index];
        else if (argument == "--symbol" && index + 1 < argc) symbols.push_back(argv[++index]);
        else if (argument == "--symbols" && index + 1 < argc) symbols = split_symbols(argv[++index]);
        else if (argument == "--period" && index + 1 < argc) period = argv[++index];
        else if (argument == "--interval" && index + 1 < argc) interval = argv[++index];
        else if (argument == "--interval-seconds" && index + 1 < argc) interval_seconds = std::max(1, std::stoi(argv[++index]));
        else if (argument == "--record" && index + 1 < argc) output = argv[++index];
        else if (argument == "--replay" && index + 1 < argc) replay_path = argv[++index];
    }
    if (mode == "replay") {
        if (replay_path.empty()) {
            std::cerr << "replay mode requires --replay session.json\n";
            return 2;
        }
        const auto session = synthia::finance::FinancialIngestionSession::load(replay_path);
        synthia::knowledge::WorldState world;
        const auto report = session.replay(world);
        std::cout << "mode=replay session=" << session.id << " accepted=" << report.accepted << " rejected=" << report.rejected << '\n';
        return report.accepted == session.records.size() ? 0 : 1;
    }
    if (mode == "streaming") {
        if (provider != "finnhub") {
            std::cerr << "streaming mode requires SYNTHIA_FINANCE_PROVIDER=finnhub or --provider finnhub\n";
            return 2;
        }
        synthia::finance::FinnhubStreamConfig config;
        config.endpoint = environment("SYNTHIA_FINANCE_ENDPOINT", "wss://ws.finnhub.io");
        config.api_key = environment("SYNTHIA_FINANCE_API_KEY");
        config.symbols = symbols;
        if (config.symbols.empty()) config.symbols = split_symbols(environment("SYNTHIA_FINANCE_SYMBOLS"));
        synthia::knowledge::WorldState world;
        synthia::finance::FinancialDataFabric fabric(world);
        synthia::finance::FinnhubWebSocketSource source(std::move(config));
        std::signal(SIGINT, stop_polling);
        std::signal(SIGTERM, stop_polling);
        const auto connected = source.run([&fabric](const synthia::finance::FinancialEvent& event) {
            const auto report = fabric.ingest_event(event);
            if (report.accepted == 0 && report.rejected > 0) std::cerr << "rejected canonical stream event\n";
        }, stopped);
        std::cerr << "provider=finnhub status=" << synthia::finance::stream_connection_status_name(source.status())
                  << " messages=" << source.report().messages << " events=" << source.report().events
                  << " malformed=" << source.report().malformed_messages << " reconnects=" << source.report().reconnects << '\n';
        if (!source.error().empty()) std::cerr << source.error() << '\n';
        if (!output.empty()) {
            const auto report = synthia::finance::IngestReport{source.report().events, fabric.observation_history().size(), 0, 0};
            synthia::finance::FinancialIngestionSession::capture("finnhub_stream", fabric, report).save(output);
        }
        return connected ? 0 : 1;
    }
    if (symbols.empty() || provider != "yahoo" || (mode != "snapshot" && mode != "polling")) {
        std::cerr << "usage: synthia_finance [--provider yahoo --mode snapshot|polling] --symbols AAPL,MSFT [--period 1d] [--interval 1m] [--interval-seconds 15] [--record session.json]\n"
                     "       SYNTHIA_FINANCE_PROVIDER=finnhub SYNTHIA_FINANCE_API_KEY=... SYNTHIA_FINANCE_SYMBOLS=AAPL,MSFT SYNTHIA_FINANCE_MODE=streaming ./build/synthia_finance\n"
                     "       ./build/synthia_finance --mode replay --replay session.json\n";
        return 2;
    }
    synthia::knowledge::WorldState world;
    synthia::finance::FinancialDataFabric fabric(world);
    synthia::finance::IngestReport report;
    std::signal(SIGINT, stop_polling);
    std::signal(SIGTERM, stop_polling);
    do {
        bool failed = false;
        for (const auto& symbol : symbols) {
            synthia::finance::YahooFinanceSource source(symbol, "US", "NASDAQ", "USD", period, interval);
            const auto current = fabric.ingest(source);
            report.received += current.received;
            report.accepted += current.accepted;
            report.rejected += current.rejected;
            report.deduplicated += current.deduplicated;
            if (!source.error().empty()) {
                failed = true;
                std::cerr << symbol << ": " << source.error() << '\n';
            }
        }
        std::cout << "provider=" << provider << " mode=" << mode << " symbols=" << symbols.size()
                  << " received=" << report.received << " accepted=" << report.accepted
                  << " rejected=" << report.rejected << " deduplicated=" << report.deduplicated << '\n';
        if (mode == "snapshot") break;
        std::this_thread::sleep_for(std::chrono::seconds(failed ? std::min(interval_seconds * 2, 120) : interval_seconds));
    } while (!stopped);
    if (!output.empty()) synthia::finance::FinancialIngestionSession::capture("cli_session", fabric, report).save(output);
    return report.accepted == 0 ? 1 : 0;
}
