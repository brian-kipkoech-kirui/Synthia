#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "synthia/finance/FinancialDataSource.hpp"
#include "synthia/knowledge/WorldState.hpp"

namespace synthia::finance {

struct IngestReport {
    std::size_t received = 0;
    std::size_t accepted = 0;
    std::size_t rejected = 0;
    std::size_t deduplicated = 0;
};

struct MarketSnapshot {
    Instrument instrument;
    double last_price = 0.0;
    double bid = 0.0;
    double ask = 0.0;
    double volume = 0.0;
    double spread = 0.0;
    double mid_price = 0.0;
    double price_change = 0.0;
    double realized_volume = 0.0;
    std::int64_t event_timestamp = 0;
    std::int64_t received_timestamp = 0;
    std::int64_t latency_ms = 0;
    std::string source_id;
    DataQuality quality = DataQuality::Invalid;
};

class FinancialDataFabric {
public:
    explicit FinancialDataFabric(knowledge::WorldState& world_state);

    void add_source(std::unique_ptr<FinancialDataSource> source);
    IngestReport ingest();
    IngestReport ingest(FinancialDataSource& source);
    IngestReport ingest_event(const FinancialEvent& event);

    const std::map<std::string, FinancialRecord>& records() const;
    const std::vector<FinancialRecord>& observation_history() const;
    const std::map<std::string, MarketSnapshot>& current_market_state() const;
    const std::vector<std::string>& rejection_reasons() const;

private:
    bool valid(const FinancialRecord& record) const;
    void apply_to_world(const FinancialRecord& record, bool current_market_record);
    bool accept_record(const FinancialRecord& record, IngestReport& report);
    bool update_market_state(const MarketRecord& record);

    knowledge::WorldState& world_state_;
    std::vector<std::unique_ptr<FinancialDataSource>> sources_;
    std::map<std::string, FinancialRecord> records_;
    std::vector<FinancialRecord> observation_history_;
    std::map<std::string, MarketSnapshot> current_market_state_;
    std::vector<std::string> rejection_reasons_;
};

}  // namespace synthia::finance
