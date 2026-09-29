#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace synthia::finance {

enum class AssetClass { Equity, Etf, Bond, Fx, Commodity, Crypto, Index, Unknown };
enum class DatasetKind { Market, Fundamental, Macro, News };
enum class MarketStatus { Open, Closed, Halted, Unknown };
enum class DataQuality { Verified, Delayed, Historical, Estimated, Stale, Invalid };
enum class FinancialEventType { Trade, Quote, PriceUpdate, VolumeUpdate };

struct Instrument {
    std::string identifier;
    std::string symbol;
    std::string name;
    std::string country;
    std::string exchange;
    std::string currency;
    std::string timezone;
    std::string sector;
    AssetClass asset_class = AssetClass::Unknown;
    std::string provider_symbol;
};

struct Provenance {
    std::string source_id;
    std::string source_uri;
    std::int64_t observed_at = 0;
    std::int64_t received_at = 0;
    DataQuality quality = DataQuality::Invalid;
    std::int64_t latency_ms = 0;
    std::int64_t source_timestamp = 0;
};

struct MarketRecord {
    Instrument instrument;
    double price = 0.0;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    double volume = 0.0;
    double bid = 0.0;
    double ask = 0.0;
    double volatility = 0.0;
    FinancialEventType event_type = FinancialEventType::PriceUpdate;
    MarketStatus market_status = MarketStatus::Unknown;
    Provenance provenance;
};

struct FinancialEvent {
    FinancialEventType type = FinancialEventType::PriceUpdate;
    Instrument instrument;
    double price = 0.0;
    double quantity = 0.0;
    double bid = 0.0;
    double ask = 0.0;
    std::string currency;
    std::int64_t event_timestamp = 0;
    std::int64_t source_timestamp = 0;
    std::int64_t received_timestamp = 0;
    std::string source_id;
    std::string source_uri;
    DataQuality quality = DataQuality::Invalid;
};

struct FundamentalRecord {
    Instrument instrument;
    std::string metric;
    double value = 0.0;
    std::string period;
    Provenance provenance;
};

struct MacroRecord {
    std::string indicator;
    std::string country;
    std::string currency;
    double value = 0.0;
    std::string period;
    Provenance provenance;
};

struct NewsRecord {
    std::string id;
    std::string headline;
    std::string language;
    std::vector<std::string> instrument_ids;
    double sentiment = 0.0;
    std::vector<std::string> topics;
    Provenance provenance;
};

using FinancialRecord = std::variant<MarketRecord, FundamentalRecord, MacroRecord, NewsRecord>;

DatasetKind kind_of(const FinancialRecord& record);
std::string asset_class_name(AssetClass asset_class);
std::string data_quality_name(DataQuality quality);
std::string financial_event_type_name(FinancialEventType type);
std::string record_key(const FinancialRecord& record);

}  // namespace synthia::finance
