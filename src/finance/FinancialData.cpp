#include "synthia/finance/FinancialData.hpp"

#include <sstream>
#include <type_traits>

namespace synthia::finance {

DatasetKind kind_of(const FinancialRecord& record) {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, MarketRecord>) return DatasetKind::Market;
        if constexpr (std::is_same_v<T, FundamentalRecord>) return DatasetKind::Fundamental;
        if constexpr (std::is_same_v<T, MacroRecord>) return DatasetKind::Macro;
        return DatasetKind::News;
    }, record);
}

std::string asset_class_name(AssetClass asset_class) {
    switch (asset_class) {
        case AssetClass::Equity: return "equity";
        case AssetClass::Etf: return "etf";
        case AssetClass::Bond: return "bond";
        case AssetClass::Fx: return "fx";
        case AssetClass::Commodity: return "commodity";
        case AssetClass::Crypto: return "crypto";
        case AssetClass::Index: return "index";
        default: return "unknown";
    }
}

std::string data_quality_name(DataQuality quality) {
    switch (quality) {
        case DataQuality::Verified: return "verified";
        case DataQuality::Delayed: return "delayed";
        case DataQuality::Historical: return "historical";
        case DataQuality::Estimated: return "estimated";
        case DataQuality::Stale: return "stale";
        default: return "invalid";
    }
}

std::string financial_event_type_name(FinancialEventType type) {
    switch (type) {
        case FinancialEventType::Trade: return "trade";
        case FinancialEventType::Quote: return "quote";
        case FinancialEventType::PriceUpdate: return "price_update";
        case FinancialEventType::VolumeUpdate: return "volume_update";
    }
    return "unknown";
}

std::string record_key(const FinancialRecord& record) {
    std::ostringstream key;
    std::visit([&key](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, MarketRecord>) {
            const auto event_key = value.provenance.source_timestamp > 0 ? value.provenance.source_timestamp : value.provenance.observed_at;
            key << "market|" << value.instrument.identifier << '|' << financial_event_type_name(value.event_type) << '|' << event_key;
        } else if constexpr (std::is_same_v<T, FundamentalRecord>) {
            key << "fundamental|" << value.instrument.identifier << '|' << value.metric << '|' << value.period;
        } else if constexpr (std::is_same_v<T, MacroRecord>) {
            key << "macro|" << value.country << '|' << value.indicator << '|' << value.period;
        } else {
            key << "news|" << value.id;
        }
    }, record);
    return key.str();
}

}  // namespace synthia::finance
