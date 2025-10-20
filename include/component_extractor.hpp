#pragma once

#include "types.hpp"
#include <vector>
#include <string>

namespace nameanalyzer {

/// Extract onset/nucleus/coda components from syllables
ComponentAnalysis analyze_components(const std::vector<std::string>& words);

} // namespace nameanalyzer
