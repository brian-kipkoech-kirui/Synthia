#include "synthia/finance/FinancialSources.hpp"

#include <cassert>
#include <fstream>

int main() {
    const char* path = "/tmp/synthia_finance_fixture.csv";
    std::ofstream fixture(path);
    fixture << "AAPL,US,NASDAQ,USD,1700000000,180,185,179,183,1000\n";
    fixture.close();
    synthia::finance::FileFinancialSource source({"fixture", "fixture", "file://fixture", "market"}, path);
    const auto records = source.fetch();
    assert(records.size() == 1);
    const auto* market = std::get_if<synthia::finance::MarketRecord>(&records.front());
    assert(market != nullptr);
    assert(market->instrument.country == "US");
    assert(market->instrument.currency == "USD");
    assert(market->provenance.quality == synthia::finance::DataQuality::Historical);
    return 0;
}
