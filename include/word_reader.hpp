#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace nameanalyzer {

/// Read words from a UTF-8 text file (one word per line)
/// Returns vector of lowercase words, filtered by minimum length
std::vector<std::string> read_words(std::string_view filename, int min_length = 2);

/// Convert string to lowercase (ASCII only for simplicity)
std::string to_lowercase(std::string_view str);

} // namespace nameanalyzer
