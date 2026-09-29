#include "synthia/finance/FinancialDataFabric.hpp"

#include <cmath>
#include <sstream>
#include <type_traits>
#include <regex>

namespace synthia::finance {

namespace {
std::string entity_id(const FinancialRecord& record) {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, MarketRecord> || std::is_same_v<T, FundamentalRecord>) return value.instrument.identifier;
        else if constexpr (std::is_same_v<T, MacroRecord>) return "macro:" + value.country + ":" + value.indicator;
        else return "news:" + value.id;
    }, record);
}

std::string content(const FinancialRecord& record) {
    std::ostringstream out;
    std::visit([&out](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, MarketRecord>) {
            out << "market " << value.instrument.symbol << " type=" << financial_event_type_name(value.event_type)
                << " price=" << value.price << " bid=" << value.bid << " ask=" << value.ask
                << " volume=" << value.volume << " volatility=" << value.volatility;
        } else if constexpr (std::is_same_v<T, FundamentalRecord>) {
            out << "fundamental " << value.instrument.symbol << ' ' << value.metric << '=' << value.value << " period=" << value.period;
        } else if constexpr (std::is_same_v<T, MacroRecord>) {
            out << "macro " << value.country << ' ' << value.indicator << '=' << value.value << " period=" << value.period;
        } else {
            out << "news " << value.headline << " sentiment=" << value.sentiment;
        }
    }, record);
    return out.str();
}

const Provenance& provenance(const FinancialRecord& record) {
    return std::visit([](const auto& value) -> const Provenance& { return value.provenance; }, record);
}
}  // namespace

FinancialDataFabric::FinancialDataFabric(knowledge::WorldState& world_state) : world_state_(world_state) {}

void FinancialDataFabric::add_source(std::unique_ptr<FinancialDataSource> source) {
    if (source) sources_.push_back(std::move(source));
}

IngestReport FinancialDataFabric::ingest() {
    IngestReport report;
    for (const auto& source : sources_) {
        const auto result = ingest(*source);
        report.received += result.received;
        report.accepted += result.accepted;
        report.rejected += result.rejected;
        report.deduplicated += result.deduplicated;
    }
    return report;
}

IngestReport FinancialDataFabric::ingest(FinancialDataSource& source) {
    IngestReport report;
    const auto source_metadata = source.source();
    world_state_.add_source(source_metadata);
    try {
        for (auto record : source.fetch()) {
            auto& record_provenance = std::visit([](auto& value) -> Provenance& { return value.provenance; }, record);
            if (record_provenance.source_id.empty()) record_provenance.source_id = source_metadata.id;
            if (record_provenance.source_uri.empty()) record_provenance.source_uri = source_metadata.uri;
            accept_record(record, report);
        }
        for (auto event : source.fetch_events()) {
            if (event.source_id.empty()) event.source_id = source_metadata.id;
            if (event.source_uri.empty()) event.source_uri = source_metadata.uri;
            ++report.received;
            auto event_report = ingest_event(event);
            report.accepted += event_report.accepted;
            report.rejected += event_report.rejected;
            report.deduplicated += event_report.deduplicated;
        }
    } catch (const std::exception& error) {
        rejection_reasons_.push_back(source_metadata.id + ": source failure: " + error.what());
    } catch (...) {
        rejection_reasons_.push_back(source_metadata.id + ": source failure: unknown exception");
    }
    return report;
}

const std::map<std::string, FinancialRecord>& FinancialDataFabric::records() const { return records_; }
const std::vector<FinancialRecord>& FinancialDataFabric::observation_history() const { return observation_history_; }
const std::map<std::string, MarketSnapshot>& FinancialDataFabric::current_market_state() const { return current_market_state_; }
const std::vector<std::string>& FinancialDataFabric::rejection_reasons() const { return rejection_reasons_; }

IngestReport FinancialDataFabric::ingest_event(const FinancialEvent& event) {
    if (!event.source_id.empty()) {
        world_state_.add_source({event.source_id, event.source_id, event.source_uri, "market"});
    }
    const bool event_fields_valid = [&event](FinancialEventType type) {
        switch (type) {
            case FinancialEventType::Trade:
                return std::isfinite(event.price) && event.price > 0.0 && std::isfinite(event.quantity) && event.quantity > 0.0;
            case FinancialEventType::Quote:
                return std::isfinite(event.bid) && std::isfinite(event.ask) && event.bid > 0.0 && event.ask >= event.bid;
            case FinancialEventType::PriceUpdate:
                return std::isfinite(event.price) && event.price > 0.0;
            case FinancialEventType::VolumeUpdate:
                return std::isfinite(event.quantity) && event.quantity >= 0.0;
        }
        return false;
    }(event.type);
    if (!event_fields_valid) {
        IngestReport report;
        report.received = 1;
        report.rejected = 1;
        rejection_reasons_.push_back(event.source_id + ": invalid " + financial_event_type_name(event.type) + " event fields");
        return report;
    }
    MarketRecord record;
    record.instrument = event.instrument;
    if (record.instrument.currency.empty()) record.instrument.currency = event.currency;
    record.event_type = event.type;
    record.price = event.price;
    record.open = event.price;
    record.high = event.price;
    record.low = event.price;
    record.close = event.price;
    record.volume = event.quantity;
    record.bid = event.bid;
    record.ask = event.ask;
    if (event.type == FinancialEventType::Quote && event.bid > 0.0 && event.ask > 0.0) {
        record.price = (event.bid + event.ask) / 2.0;
        record.open = record.high = record.low = record.close = record.price;
    }
    if (event.type == FinancialEventType::VolumeUpdate) {
        record.price = 0.0;
        record.open = record.high = record.low = record.close = 0.0;
    }
    record.provenance = {event.source_id, event.source_uri, event.event_timestamp,
        event.received_timestamp, event.quality,
        (event.received_timestamp - event.event_timestamp) * 1000,
        event.source_timestamp > 0 ? event.source_timestamp : event.event_timestamp};
    IngestReport report;
    ++report.received;
    accept_record(FinancialRecord{std::move(record)}, report);
    return report;
}

bool FinancialDataFabric::accept_record(const FinancialRecord& record, IngestReport& report) {
    ++report.received;
    if (!valid(record)) {
        ++report.rejected;
        rejection_reasons_.push_back(record_key(record) + ": invalid record or provenance");
        return false;
    }
    const auto key = record_key(record);
    if (records_.find(key) != records_.end()) {
        ++report.deduplicated;
        return false;
    }
    records_.emplace(key, record);
    observation_history_.push_back(record);
    bool current_market_record = true;
    if (const auto* market = std::get_if<MarketRecord>(&record)) current_market_record = update_market_state(*market);
    apply_to_world(record, current_market_record);
    ++report.accepted;
    return true;
}

bool FinancialDataFabric::update_market_state(const MarketRecord& record) {
    const auto& provenance = record.provenance;
    const auto key = record.instrument.symbol;
    auto& state = current_market_state_[key];
    if (provenance.observed_at < state.event_timestamp) return false;
    if (state.instrument.symbol.empty()) state.instrument = record.instrument;
    if (record.event_type == FinancialEventType::Trade || record.event_type == FinancialEventType::PriceUpdate) {
        if (record.price > 0.0) {
            state.price_change = state.last_price == 0.0 ? 0.0 : record.price - state.last_price;
            state.last_price = record.price;
        }
        if (record.event_type == FinancialEventType::PriceUpdate) state.volume = record.volume;
        else state.realized_volume += record.volume;
    } else if (record.event_type == FinancialEventType::Quote) {
        state.bid = record.bid;
        state.ask = record.ask;
        state.spread = record.ask - record.bid;
        state.mid_price = (record.bid + record.ask) / 2.0;
        if (state.mid_price > 0.0) {
            state.price_change = state.last_price == 0.0 ? 0.0 : state.mid_price - state.last_price;
            state.last_price = state.mid_price;
        }
    } else {
        state.realized_volume += record.volume;
    }
    if (record.bid > 0.0) state.bid = record.bid;
    if (record.ask > 0.0) state.ask = record.ask;
    if (state.bid > 0.0 && state.ask > 0.0) {
        state.spread = state.ask - state.bid;
        state.mid_price = (state.bid + state.ask) / 2.0;
    }
    state.event_timestamp = provenance.observed_at;
    state.received_timestamp = provenance.received_at;
    state.latency_ms = provenance.latency_ms;
    state.source_id = provenance.source_id;
    state.quality = provenance.quality;
    return true;
}

bool FinancialDataFabric::valid(const FinancialRecord& record) const {
    const auto& record_provenance = provenance(record);
    if (record_provenance.source_id.empty() || record_provenance.observed_at <= 0 || record_provenance.received_at < record_provenance.observed_at || record_provenance.quality == DataQuality::Invalid) return false;
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, MarketRecord>) return !value.instrument.identifier.empty() && std::regex_match(value.instrument.symbol, std::regex("[A-Za-z0-9._=^/-]+")) && std::isfinite(value.price) && value.price >= 0.0 && std::isfinite(value.open) && std::isfinite(value.high) && std::isfinite(value.low) && std::isfinite(value.close) && value.high >= value.low && value.high >= value.open && value.high >= value.close && value.low <= value.open && value.low <= value.close && std::isfinite(value.volume) && value.volume >= 0.0 && std::isfinite(value.bid) && value.bid >= 0.0 && std::isfinite(value.ask) && value.ask >= 0.0 && (value.bid == 0.0 || value.ask == 0.0 || value.ask >= value.bid);
        else if constexpr (std::is_same_v<T, FundamentalRecord>) return !value.instrument.identifier.empty() && !value.metric.empty() && std::isfinite(value.value);
        else if constexpr (std::is_same_v<T, MacroRecord>) return !value.country.empty() && !value.indicator.empty() && std::isfinite(value.value);
        else return !value.id.empty() && !value.headline.empty() && std::isfinite(value.sentiment) && value.sentiment >= -1.0 && value.sentiment <= 1.0;
    }, record);
}

void FinancialDataFabric::apply_to_world(const FinancialRecord& record, bool current_market_record) {
    const auto& record_provenance = provenance(record);
    const auto id = entity_id(record);
    knowledge::Entity entity;
    entity.id = id;
    entity.canonical_name = id;
    entity.entity_type = kind_of(record) == DatasetKind::News ? "financial_news" : "financial_observation";
    entity.description = content(record);
    if (!std::holds_alternative<MarketRecord>(record) || current_market_record || !world_state_.contains_entity(id)) {
        if (const auto market = current_market_state_.find(std::get_if<MarketRecord>(&record) ? std::get<MarketRecord>(record).instrument.symbol : ""); market != current_market_state_.end()) {
            const auto& state = market->second;
            std::ostringstream current;
            current << "market " << state.instrument.symbol << " last=" << state.last_price
                    << " bid=" << state.bid << " ask=" << state.ask << " volume=" << state.volume
                    << " updated=" << state.event_timestamp << " source=" << state.source_id
                    << " quality=" << data_quality_name(state.quality);
            entity.description = current.str();
        }
        world_state_.add_entity(entity);
    }

    knowledge::Observation observation;
    observation.id = "financial:" + record_key(record);
    observation.timestamp = record_provenance.observed_at;
    observation.source_id = record_provenance.source_id;
    observation.content = content(record);
    observation.data_type = data_quality_name(record_provenance.quality);
    observation.confidence = record_provenance.quality == DataQuality::Verified ? 1.0 : 0.5;
    observation.related_entity_ids = {id};
    world_state_.apply(observation);
}

}  // namespace synthia::finance
