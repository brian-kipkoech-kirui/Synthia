#include "synthia/finance/FinancialIngestionSession.hpp"

#include <cassert>
#include <cstdio>
#include <memory>

int main() {
    using namespace synthia::finance;
    synthia::knowledge::WorldState world;
    MarketRecord record;
    record.instrument = {"ke:nse:SCOM", "SCOM", "Safaricom", "KE", "NSE", "KES", "Africa/Nairobi", "telecom", AssetClass::Equity, "SCOM.NR"};
    record.open = 20; record.high = 21; record.low = 19; record.close = 20.5; record.price = 20.5; record.volume = 500;
    record.provenance = {"fixture", "file://fixture", 1700000000, 1700000010, DataQuality::Historical, 0, 1700000000};
    FinancialDataFabric fabric(world);
    fabric.add_source(std::make_unique<StaticFinancialDataSource>(synthia::knowledge::Source{"fixture", "fixture", "file://fixture", "market"}, std::vector<FinancialRecord>{record}));
    const auto report = fabric.ingest();
    const auto session = FinancialIngestionSession::capture("session_1", fabric, report);
    session.save("/tmp/synthia_session.json");
    const auto loaded = FinancialIngestionSession::load("/tmp/synthia_session.json");
    assert(loaded.records.size() == 1);
    synthia::knowledge::WorldState replay_world;
    const auto replay_report = loaded.replay(replay_world);
    assert(replay_report.accepted == 1);
    assert(replay_world.observations().size() == 1);
    assert(replay_world.observations().begin()->second.timestamp == 1700000000);
    const auto& replayed_record = std::get<MarketRecord>(loaded.records.front());
    assert(replayed_record.provenance.received_at == 1700000010);
    assert(replayed_record.provenance.source_uri == "file://fixture");
    assert(replayed_record.instrument.symbol == "SCOM");
    assert(replayed_record.price == 20.5);
    assert(replayed_record.volume == 500.0);

    MarketRecord quote = record;
    quote.event_type = FinancialEventType::Quote;
    quote.provenance.observed_at = 1700000001;
    quote.provenance.received_at = 1700000011;
    quote.bid = 20.4;
    quote.ask = 20.6;
    const auto ordered = FinancialIngestionSession{"ordered_session", 1, 2, {}, {record, quote}, {}, {}};
    ordered.save("/tmp/synthia_ordered_session.json");
    const auto ordered_loaded = FinancialIngestionSession::load("/tmp/synthia_ordered_session.json");
    assert(ordered_loaded.records.size() == 2);
    assert(std::get<MarketRecord>(ordered_loaded.records[0]).event_type == FinancialEventType::PriceUpdate);
    assert(std::get<MarketRecord>(ordered_loaded.records[1]).event_type == FinancialEventType::Quote);
    assert(std::get<MarketRecord>(ordered_loaded.records[1]).bid == 20.4);
    assert(std::get<MarketRecord>(ordered_loaded.records[1]).ask == 20.6);
    std::remove("/tmp/synthia_ordered_session.json");
    std::remove("/tmp/synthia_session.json");
    return 0;
}
