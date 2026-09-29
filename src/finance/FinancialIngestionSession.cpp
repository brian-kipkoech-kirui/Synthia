#include "synthia/finance/FinancialIngestionSession.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

namespace synthia::finance {
namespace {
std::int64_t now() { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
std::string quote(const std::string& value) {
    std::ostringstream output;
    output << '"';
    for (const unsigned char character : value) {
        switch (character) {
            case '\\': output << "\\\\"; break;
            case '"': output << "\\\""; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default: output << character; break;
        }
    }
    output << '"';
    return output.str();
}

std::string unquote(const std::string& value) {
    std::string result;
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1 == value.size()) {
            result.push_back(value[index]);
            continue;
        }
        const char escaped = value[++index];
        if (escaped == 'n') result.push_back('\n');
        else if (escaped == 'r') result.push_back('\r');
        else if (escaped == 't') result.push_back('\t');
        else result.push_back(escaped);
    }
    return result;
}

DataQuality parse_quality(const std::string& value) {
    if (value == "verified") return DataQuality::Verified;
    if (value == "delayed") return DataQuality::Delayed;
    if (value == "historical") return DataQuality::Historical;
    if (value == "estimated") return DataQuality::Estimated;
    if (value == "stale") return DataQuality::Stale;
    return DataQuality::Invalid;
}

AssetClass parse_asset_class(const std::string& value) {
    if (value == "equity") return AssetClass::Equity;
    if (value == "etf") return AssetClass::Etf;
    if (value == "bond") return AssetClass::Bond;
    if (value == "fx") return AssetClass::Fx;
    if (value == "commodity") return AssetClass::Commodity;
    if (value == "crypto") return AssetClass::Crypto;
    if (value == "index") return AssetClass::Index;
    return AssetClass::Unknown;
}
}

FinancialIngestionSession FinancialIngestionSession::capture(const std::string& id, FinancialDataFabric& fabric, IngestReport report) {
    FinancialIngestionSession session; session.id = id; session.started_at = now(); session.ended_at = now(); session.report = report;
    session.records = fabric.observation_history();
    std::set<std::string> providers;
    for (const auto& record : session.records) {
        const auto& record_provenance = std::visit([](const auto& value) -> const Provenance& { return value.provenance; }, record);
        providers.insert(record_provenance.source_id);
    }
    session.providers.assign(providers.begin(), providers.end());
    return session;
}

void FinancialIngestionSession::save(const std::string& path) const {
    std::ofstream file(path);
    if (!file) throw std::runtime_error("unable to open financial session for writing: " + path);
    file << "{\"id\":" << quote(id) << ",\"started_at\":" << started_at << ",\"ended_at\":" << ended_at
         << ",\"received\":" << report.received << ",\"accepted\":" << report.accepted << ",\"rejected\":" << report.rejected << ",\"deduplicated\":" << report.deduplicated << ",\"records\":[";
    bool first = true;
    for (const auto& record : records) {
        if (!first) file << ','; first = false;
        if (const auto* market = std::get_if<MarketRecord>(&record)) {
            file << std::setprecision(std::numeric_limits<double>::max_digits10)
                 << "{\"type\":\"market\",\"identifier\":" << quote(market->instrument.identifier)
                 << ",\"symbol\":" << quote(market->instrument.symbol) << ",\"name\":" << quote(market->instrument.name)
                 << ",\"country\":" << quote(market->instrument.country) << ",\"exchange\":" << quote(market->instrument.exchange)
                 << ",\"currency\":" << quote(market->instrument.currency) << ",\"timezone\":" << quote(market->instrument.timezone)
                 << ",\"sector\":" << quote(market->instrument.sector) << ",\"asset_class\":" << quote(asset_class_name(market->instrument.asset_class))
                 << ",\"provider_symbol\":" << quote(market->instrument.provider_symbol)
                 << ",\"observed_at\":" << market->provenance.observed_at << ",\"received_at\":" << market->provenance.received_at
                 << ",\"source_id\":" << quote(market->provenance.source_id) << ",\"source_uri\":" << quote(market->provenance.source_uri)
                 << ",\"quality\":" << quote(data_quality_name(market->provenance.quality))
                 << ",\"latency_ms\":" << market->provenance.latency_ms << ",\"source_timestamp\":" << market->provenance.source_timestamp
                 << ",\"event_type\":" << quote(financial_event_type_name(market->event_type))
                 << ",\"price\":" << market->price << ",\"open\":" << market->open << ",\"high\":" << market->high
                 << ",\"low\":" << market->low << ",\"close\":" << market->close << ",\"volume\":" << market->volume
                 << ",\"bid\":" << market->bid << ",\"ask\":" << market->ask << ",\"volatility\":" << market->volatility << '}';
        }
    }
    file << "]}\n";
}

FinancialIngestionSession FinancialIngestionSession::load(const std::string& path) {
    FinancialIngestionSession session; std::ifstream file(path);
    if (!file) throw std::runtime_error("unable to open financial session: " + path);
    std::stringstream input; input << file.rdbuf(); const auto json = input.str();
    std::smatch match;
    if (std::regex_search(json, match, std::regex(R"json("id":"((?:\\.|[^"\\])*)")json"))) session.id = unquote(match[1]);
    const std::regex object_regex(R"(\{[^{}]*\})");
    const std::regex field_regex(R"json("([a-z_]+)":(?:"((?:\\.|[^"\\])*)"|(-?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)))json");
    for (auto object = std::sregex_iterator(json.begin(), json.end(), object_regex); object != std::sregex_iterator(); ++object) {
        const auto text = object->str();
        if (text.find("\"type\":\"market\"") == std::string::npos) continue;
        std::map<std::string, std::string> fields;
        for (auto field = std::sregex_iterator(text.begin(), text.end(), field_regex); field != std::sregex_iterator(); ++field) {
            fields[(*field)[1].str()] = (*field)[2].matched ? unquote((*field)[2].str()) : (*field)[3].str();
        }
        const auto string_field = [&fields](const std::string& name) { return fields[name]; };
        const auto number_field = [&fields](const std::string& name) { return fields.count(name) ? std::stod(fields[name]) : 0.0; };
        MarketRecord record;
        record.instrument.identifier = string_field("identifier"); record.instrument.symbol = string_field("symbol");
        record.instrument.name = string_field("name"); record.instrument.country = string_field("country");
        record.instrument.exchange = string_field("exchange"); record.instrument.currency = string_field("currency");
        record.instrument.timezone = string_field("timezone"); record.instrument.sector = string_field("sector");
        record.instrument.asset_class = parse_asset_class(string_field("asset_class"));
        record.instrument.provider_symbol = string_field("provider_symbol");
        record.price = number_field("price"); record.open = number_field("open"); record.high = number_field("high");
        record.low = number_field("low"); record.close = number_field("close"); record.volume = number_field("volume");
        record.bid = number_field("bid"); record.ask = number_field("ask"); record.volatility = number_field("volatility");
        const auto event_type = string_field("event_type");
        if (event_type == "trade") record.event_type = FinancialEventType::Trade;
        else if (event_type == "quote") record.event_type = FinancialEventType::Quote;
        else if (event_type == "volume_update") record.event_type = FinancialEventType::VolumeUpdate;
        const auto observed = static_cast<std::int64_t>(number_field("observed_at"));
        const auto received = static_cast<std::int64_t>(number_field("received_at"));
        record.provenance = {string_field("source_id"), string_field("source_uri"), observed, received,
            parse_quality(string_field("quality")), static_cast<std::int64_t>(number_field("latency_ms")),
            static_cast<std::int64_t>(number_field("source_timestamp"))};
        session.records.emplace_back(std::move(record));
    }
    session.report.received = session.records.size(); session.report.accepted = session.records.size(); return session;
}

IngestReport FinancialIngestionSession::replay(knowledge::WorldState& world) const {
    if (records.empty()) return {};
    for (const auto& record : records) {
        const auto& record_provenance = std::visit([](const auto& value) -> const Provenance& { return value.provenance; }, record);
        world.add_source({record_provenance.source_id, record_provenance.source_id, record_provenance.source_uri, "market"});
    }
    const auto& provenance = std::visit([](const auto& value) -> const Provenance& { return value.provenance; }, records.front());
    StaticFinancialDataSource source({provenance.source_id, "Recorded ingestion session", "session://" + id, "replay"}, records);
    FinancialDataFabric fabric(world); return fabric.ingest(source);
}
}  // namespace synthia::finance
