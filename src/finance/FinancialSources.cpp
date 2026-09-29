#include "synthia/finance/FinancialSources.hpp"

#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <regex>
#include <set>
#include <sstream>

namespace synthia::finance {
namespace {
std::int64_t now() { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
std::string shell_quote(const std::string& value) { return "'" + std::regex_replace(value, std::regex("'"), "'\\\"'\\\"'") + "'"; }
std::string command_output(const std::string& command, int& status) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) { status = -1; return {}; }
    char buffer[4096]; std::string output;
    while (fgets(buffer, sizeof(buffer), pipe)) output += buffer;
    status = pclose(pipe);
    return output;
}
std::vector<std::int64_t> integers(const std::string& json, const std::string& name) {
    std::smatch match;
    if (!std::regex_search(json, match, std::regex("\\\"" + name + "\\\"\\s*:\\s*\\[([^]]*)\\]"))) return {};
    std::vector<std::int64_t> values; std::regex number("-?[0-9]+(?:\\.[0-9]+)?");
    for (auto it = std::sregex_iterator(match[1].first, match[1].second, number); it != std::sregex_iterator(); ++it) values.push_back(static_cast<std::int64_t>(std::stod(it->str())));
    return values;
}
std::vector<double> numbers(const std::string& json, const std::string& name) {
    std::smatch match;
    if (!std::regex_search(json, match, std::regex("\\\"" + name + "\\\"\\s*:\\s*\\[([^]]*)\\]"))) return {};
    std::vector<double> values;
    std::stringstream array(match[1]);
    std::string token;
    while (std::getline(array, token, ',')) {
        const auto first = token.find_first_not_of(" \t\r\n");
        const auto last = token.find_last_not_of(" \t\r\n");
        if (first == std::string::npos || token.substr(first, last - first + 1) == "null") {
            values.push_back(std::numeric_limits<double>::quiet_NaN());
        } else {
            try { values.push_back(std::stod(token.substr(first, last - first + 1))); }
            catch (const std::exception&) { values.push_back(std::numeric_limits<double>::quiet_NaN()); }
        }
    }
    return values;
}
std::string csv_field(std::stringstream& line) { std::string value; std::getline(line, value, ','); return value; }
}

YahooFinanceSource::YahooFinanceSource(std::string symbol, std::string country, std::string exchange, std::string currency, std::string period, std::string interval)
    : symbol_(std::move(symbol)), country_(std::move(country)), exchange_(std::move(exchange)), currency_(std::move(currency)), period_(std::move(period)), interval_(std::move(interval)) {}

knowledge::Source YahooFinanceSource::source() const { return {"yahoo_finance", "Yahoo Finance chart API", "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol_, "market"}; }
const std::string& YahooFinanceSource::error() const { return error_; }

std::vector<FinancialRecord> YahooFinanceSource::fetch() {
    error_.clear();
    if (!std::regex_match(symbol_, std::regex("[A-Za-z0-9.^=_-]+"))) {
        error_ = "Yahoo Finance symbol contains unsupported characters";
        return {};
    }
    const std::set<std::string> supported_periods{"1d", "5d", "1mo", "3mo", "6mo", "1y", "2y", "5y", "10y", "ytd", "max"};
    const std::set<std::string> supported_intervals{"1m", "2m", "5m", "15m", "30m", "60m", "90m", "1h", "1d", "5d", "1wk", "1mo", "3mo"};
    if (supported_periods.count(period_) == 0 || supported_intervals.count(interval_) == 0) {
        error_ = "Yahoo Finance period or interval is unsupported";
        return {};
    }
    const auto intraday_intervals = {"1m", "2m", "5m", "15m", "30m", "60m", "90m", "1h"};
    const bool intraday = std::find(intraday_intervals.begin(), intraday_intervals.end(), interval_) != intraday_intervals.end();
    const std::string effective_interval = period_ != "1d" && period_ != "5d" && intraday ? "1d" : interval_;
    const auto received = now();
    const std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol_ + "?range=" + period_ + "&interval=" + effective_interval;
    int status = 0;
    const auto response = command_output("curl --fail --silent --show-error --max-time 15 " + shell_quote(url), status);
    if (status != 0 || response.empty()) { error_ = "Yahoo Finance request failed"; return {}; }
    const auto timestamps = integers(response, "timestamp");
    const auto opens = numbers(response, "open"); const auto highs = numbers(response, "high");
    const auto lows = numbers(response, "low"); const auto closes = numbers(response, "close"); const auto volumes = numbers(response, "volume");
    if (timestamps.empty() || closes.empty()) { error_ = "Yahoo Finance response did not contain chart data"; return {}; }
    std::vector<FinancialRecord> records;
    const auto count = std::min({timestamps.size(), opens.size(), highs.size(), lows.size(), closes.size(), volumes.size()});
    for (std::size_t index = 0; index < count; ++index) {
        if (!std::isfinite(closes[index]) || closes[index] <= 0.0 || !std::isfinite(opens[index]) || !std::isfinite(highs[index]) || !std::isfinite(lows[index]) || !std::isfinite(volumes[index])) continue;
        MarketRecord record;
        record.instrument = {country_ + ":" + exchange_ + ":" + symbol_, symbol_, symbol_, country_, exchange_, currency_, "UTC", "", AssetClass::Equity, symbol_};
        record.open = opens[index]; record.high = highs[index]; record.low = lows[index]; record.close = closes[index]; record.price = closes[index]; record.volume = volumes[index];
        record.market_status = MarketStatus::Unknown;
        record.provenance = {source().id, source().uri, timestamps[index], received, DataQuality::Delayed, (received - timestamps[index]) * 1000, timestamps[index]};
        records.emplace_back(std::move(record));
    }
    if (records.empty()) error_ = "Yahoo Finance response contained no valid observations";
    return records;
}

FileFinancialSource::FileFinancialSource(knowledge::Source source, std::string path) : source_(std::move(source)), path_(std::move(path)) {}
knowledge::Source FileFinancialSource::source() const { return source_; }
const std::string& FileFinancialSource::error() const { return error_; }

std::vector<FinancialRecord> FileFinancialSource::fetch() {
    error_.clear(); std::ifstream file(path_); if (!file) { error_ = "unable to open financial fixture"; return {}; }
    std::vector<FinancialRecord> records; std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream fields(line); const auto symbol = csv_field(fields); const auto country = csv_field(fields); const auto exchange = csv_field(fields); const auto currency = csv_field(fields);
        const auto observed = csv_field(fields); const auto open = csv_field(fields); const auto high = csv_field(fields); const auto low = csv_field(fields); const auto close = csv_field(fields); const auto volume = csv_field(fields);
        if (symbol.empty() || observed.empty() || close.empty()) { error_ = "malformed financial fixture row"; continue; }
        try {
            MarketRecord record; record.instrument = {country + ":" + exchange + ":" + symbol, symbol, symbol, country, exchange, currency, "UTC", "", AssetClass::Equity, symbol};
            record.open = std::stod(open); record.high = std::stod(high); record.low = std::stod(low); record.close = std::stod(close); record.price = record.close; record.volume = std::stod(volume);
            record.provenance = {source_.id, source_.uri, std::stoll(observed), now(), DataQuality::Historical, 0, std::stoll(observed)};
            records.emplace_back(std::move(record));
        } catch (const std::exception&) { error_ = "malformed numeric value in financial fixture"; }
    }
    return records;
}
}  // namespace synthia::finance
