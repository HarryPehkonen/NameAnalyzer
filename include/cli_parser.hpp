#pragma once

#include "types.hpp"
#include <optional>
#include <string_view>

namespace nameanalyzer {

/// Parse command-line arguments and return configuration
/// Returns std::nullopt if parsing fails or help is requested
std::optional<Config> parse_arguments(int argc, char* argv[]);

/// Print usage information
void print_usage(std::string_view program_name);

} // namespace nameanalyzer
