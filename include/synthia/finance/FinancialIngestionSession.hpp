#pragma once

#include <string>
#include <vector>

#include "synthia/finance/FinancialDataFabric.hpp"

namespace synthia::finance {

struct FinancialIngestionSession {
    std::string id;
    std::int64_t started_at = 0;
    std::int64_t ended_at = 0;
    std::vector<std::string> providers;
    std::vector<FinancialRecord> records;
    IngestReport report;
    std::vector<std::string> errors;

    static FinancialIngestionSession capture(const std::string& id, FinancialDataFabric& fabric, IngestReport report);
    static FinancialIngestionSession load(const std::string& path);
    void save(const std::string& path) const;
    IngestReport replay(knowledge::WorldState& world) const;
};

}  // namespace synthia::finance