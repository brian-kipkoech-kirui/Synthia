#include "synthia/finance/FinancialDataSource.hpp"

namespace synthia::finance {

StaticFinancialDataSource::StaticFinancialDataSource(knowledge::Source source, std::vector<FinancialRecord> records)
    : source_(std::move(source)), records_(std::move(records)) {}

knowledge::Source StaticFinancialDataSource::source() const { return source_; }
std::vector<FinancialRecord> StaticFinancialDataSource::fetch() { return records_; }

}  // namespace synthia::finance
