#include "synthia/finance/FinancialDataFabric.hpp"

#include <cassert>
#include <memory>

using namespace synthia::finance;
using synthia::knowledge::Source;

namespace {
MarketRecord market(const char* source_id, double price) {
    MarketRecord record;
    record.instrument = {"us:nasdaq:AAPL", "AAPL", "Apple Inc.", "US", "NASDAQ", "USD", "America/New_York", "technology", AssetClass::Equity};
    record.price = price;
    record.volume = 1000000.0;
    record.volatility = 0.22;
    record.market_status = MarketStatus::Open;
    record.provenance = {source_id, "https://example.test/market", 100, 110, DataQuality::Verified, 10};
    return record;
}
}

int main() {
    synthia::knowledge::WorldState knowledge;
    FinancialDataFabric fabric(knowledge);
    Source source{"source_exchange", "Example Exchange", "https://example.test", "market"};
    fabric.add_source(std::make_unique<StaticFinancialDataSource>(source, std::vector<FinancialRecord>{market("", 190.0), market("", 191.0)}));

    const auto first = fabric.ingest();
    assert(first.received == 2);
    assert(first.accepted == 1);
    assert(first.deduplicated == 1);
    assert(first.rejected == 0);
    assert(fabric.records().size() == 1);
    assert(knowledge.sources().size() == 1);
    assert(knowledge.entities().size() == 1);
    assert(knowledge.observations().size() == 1);
    assert(knowledge.observations().begin()->second.source_id == "source_exchange");
    assert(knowledge.observations().begin()->second.timestamp == 100);

    auto invalid = market("", 10.0);
    invalid.provenance.quality = DataQuality::Invalid;
    StaticFinancialDataSource invalid_source(source, {invalid});
    const auto second = fabric.ingest(invalid_source);
    assert(second.received == 1);
    assert(second.rejected == 1);
    assert(!fabric.rejection_reasons().empty());

    assert(record_key(market("source_a", 190.0)) == record_key(market("source_b", 191.0)));
    return 0;
}
