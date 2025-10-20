#pragma once

#include "types.hpp"
#include <string>

namespace nameanalyzer {

/// Write analysis results to JSON file using JSOM library
void write_json_output(const AnalysisResults& results, const std::string& filename);

} // namespace nameanalyzer
