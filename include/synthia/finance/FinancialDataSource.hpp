#pragma once

#include <string>
#include <vector>

#include "synthia/finance/FinancialData.hpp"
#include "synthia/knowledge/Source.hpp"

namespace synthia::finance {

enum class DataDeliveryMode { Snapshot, Polling, Streaming };

class FinancialDataSource {
public:
    virtual ~FinancialDataSource() = default;
    virtual knowledge::Source source() const = 0;
    virtual std::vector<FinancialRecord> fetch() = 0;
    virtual DataDeliveryMode delivery_mode() const { return DataDeliveryMode::Snapshot; }
    virtual std::vector<FinancialEvent> fetch_events() { return {}; }
};

class StaticFinancialDataSource final : public FinancialDataSource {
public:
    StaticFinancialDataSource(knowledge::Source source, std::vector<FinancialRecord> records);

    knowledge::Source source() const override;
    std::vector<FinancialRecord> fetch() override;

private:
    knowledge::Source source_;
    std::vector<FinancialRecord> records_;
};

}  // namespace synthia::finance
