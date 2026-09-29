#pragma once

#include <string>

#include "synthia/finance/FinancialDataSource.hpp"

namespace synthia::finance {

class YahooFinanceSource final : public FinancialDataSource {
public:
    YahooFinanceSource(std::string symbol, std::string country = "US", std::string exchange = "NASDAQ", std::string currency = "USD", std::string period = "1d", std::string interval = "1m");

    knowledge::Source source() const override;
    std::vector<FinancialRecord> fetch() override;
    DataDeliveryMode delivery_mode() const override { return DataDeliveryMode::Polling; }
    const std::string& error() const;

private:
    std::string symbol_;
    std::string country_;
    std::string exchange_;
    std::string currency_;
    std::string period_;
    std::string interval_;
    std::string error_;
};

class FileFinancialSource final : public FinancialDataSource {
public:
    FileFinancialSource(knowledge::Source source, std::string path);

    knowledge::Source source() const override;
    std::vector<FinancialRecord> fetch() override;
    const std::string& error() const;

private:
    knowledge::Source source_;
    std::string path_;
    std::string error_;
};

}  // namespace synthia::finance