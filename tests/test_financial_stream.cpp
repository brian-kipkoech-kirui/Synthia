#include "synthia/finance/FinancialDataFabric.hpp"
#include "synthia/finance/FinancialIngestionSession.hpp"

#include <cassert>
#include <cstdio>
#include <stdexcept>

using namespace synthia::finance;

namespace {
class EventSource final : public FinancialDataSource {
public:
    explicit EventSource(std::vector<FinancialEvent> events, bool fail = false)
        : events_(std::move(events)), fail_(fail) {}

    synthia::knowledge::Source source() const override {
        return {"stream_fixture", "Stream fixture", "fixture://stream", "market"};
    }
    std::vector<FinancialRecord> fetch() override { return {}; }
    DataDeliveryMode delivery_mode() const override { return DataDeliveryMode::Streaming; }
    std::vector<FinancialEvent> fetch_events() override {
        if (fail_) throw std::runtime_error("fixture disconnect");
        return events_;
    }

private:
    std::vector<FinancialEvent> events_;
    bool fail_;
};

FinancialEvent event(FinancialEventType type, std::int64_t timestamp) {
    FinancialEvent value;
    value.type = type;
    value.instrument = {"us:nasdaq:ACME", "ACME", "Acme", "US", "NASDAQ", "USD"};
    value.price = 101.25;
    value.quantity = 12.0;
    value.bid = 101.0;
    value.ask = 101.5;
    value.event_timestamp = timestamp;
    value.received_timestamp = timestamp + 2;
    value.source_id = "fixture_provider";
    value.source_uri = "fixture://market";
    value.quality = DataQuality::Verified;
    return value;
}

void source_interface() {
    EventSource source({}, false);
    assert(source.delivery_mode() == DataDeliveryMode::Streaming);
}

void stream_and_state() {
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    EventSource source({event(FinancialEventType::Quote, 100), event(FinancialEventType::Trade, 101)});
    const auto report = fabric.ingest(source);
    assert(report.received == 2 && report.accepted == 2);
    const auto& snapshot = fabric.current_market_state().at("ACME");
    assert(snapshot.last_price == 101.25);
    assert(snapshot.bid == 101.0 && snapshot.ask == 101.5);
    assert(snapshot.spread == 0.5 && snapshot.mid_price == 101.25);
    assert(snapshot.realized_volume == 12.0);
}

void event_validation() {
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    auto malformed = event(FinancialEventType::Trade, 100);
    malformed.quantity = -1.0;
    const auto report = fabric.ingest_event(malformed);
    assert(report.rejected == 1 && report.accepted == 0);
    auto invalid_quote = event(FinancialEventType::Quote, 101);
    invalid_quote.ask = 0.0;
    assert(fabric.ingest_event(invalid_quote).rejected == 1);
}

void event_deduplication() {
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    const auto value = event(FinancialEventType::Trade, 100);
    const auto first = fabric.ingest_event(value);
    const auto second = fabric.ingest_event(value);
    assert(first.accepted == 1 && second.deduplicated == 1);
    assert(fabric.records().size() == 1);
}

void provenance_projection() {
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    const auto value = event(FinancialEventType::Trade, 100);
    assert(fabric.ingest_event(value).accepted == 1);
    assert(world.sources().at("fixture_provider").uri == "fixture://market");
    const auto& observation = world.observations().begin()->second;
    assert(observation.timestamp == 100);
    assert(observation.source_id == "fixture_provider");
}

void latest_world_state() {
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    assert(fabric.ingest_event(event(FinancialEventType::Trade, 101)).accepted == 1);
    auto older = event(FinancialEventType::Trade, 99);
    older.price = 90.0;
    assert(fabric.ingest_event(older).accepted == 1);
    assert(fabric.current_market_state().at("ACME").last_price == 101.25);
    assert(world.contains_entity("us:nasdaq:ACME"));
    assert(world.get_entity("us:nasdaq:ACME")->description.find("last=101.25") != std::string::npos);
}

void source_failure() {
    synthia::knowledge::WorldState world;
    FinancialDataFabric fabric(world);
    EventSource source({}, true);
    const auto report = fabric.ingest(source);
    assert(report.accepted == 0 && report.rejected == 0);
    assert(!fabric.rejection_reasons().empty());
}

void deterministic_replay() {
    synthia::knowledge::WorldState live_world;
    FinancialDataFabric live_fabric(live_world);
    auto first = event(FinancialEventType::Trade, 1700000000);
    first.source_timestamp = 1700000000123;
    auto second = event(FinancialEventType::Trade, 1700000000);
    second.source_timestamp = 1700000000456;
    second.price = 102.0;
    assert(live_fabric.ingest_event(first).accepted == 1);
    assert(live_fabric.ingest_event(second).accepted == 1);
    const auto captured = FinancialIngestionSession::capture("stream_replay", live_fabric, {2, 2, 0, 0});
    captured.save("/tmp/synthia_stream_replay.json");
    const auto loaded = FinancialIngestionSession::load("/tmp/synthia_stream_replay.json");
    assert(loaded.records.size() == 2);
    synthia::knowledge::WorldState replay_world;
    const auto report = loaded.replay(replay_world);
    assert(report.accepted == 2);
    assert(live_fabric.observation_history().size() == 2);
    const auto live_state = live_fabric.current_market_state().at("ACME");
    const auto replay_description = replay_world.entities().at("us:nasdaq:ACME").description;
    assert(live_state.last_price == 102.0);
    assert(replay_description.find("last=102") != std::string::npos);
    assert(replay_description.find("updated=1700000000") != std::string::npos);
    assert(replay_world.observations().size() == live_world.observations().size());
    std::remove("/tmp/synthia_stream_replay.json");
}

}

int main(int argc, char** argv) {
    assert(argc == 2);
    const std::string test = argv[1];
    if (test == "source") source_interface();
    else if (test == "stream") stream_and_state();
    else if (test == "validation") event_validation();
    else if (test == "deduplication") event_deduplication();
    else if (test == "provenance") provenance_projection();
    else if (test == "world_state") latest_world_state();
    else if (test == "connection_failure") source_failure();
    else if (test == "replay") deterministic_replay();
    else assert(false && "unknown test case");
    return 0;
}