#include "cli_parser.hpp"
#include <iostream>
#include <algorithm>
#include <string_view>

namespace nameanalyzer {

void print_usage(std::string_view program_name) {
    std::cout << "NameAnalyzer - Analyze words to extract statistical patterns\n\n"
              << "Usage: " << program_name << " <input_file> -o <output_file> [options]\n\n"
              << "Required arguments:\n"
              << "  <input_file>              Input text file (one word per line, UTF-8)\n"
              << "  -o, --output <file>       Output JSON file for statistics\n\n"
              << "Options:\n"
              << "  --markov-order <1-3>      Markov chain order (default: 2)\n"
              << "  --enable-syllables        Enable syllable-level analysis\n"
              << "  --enable-components       Enable onset/nucleus/coda extraction\n"
              << "  --min-length <n>          Minimum word length to analyze (default: 2)\n"
              << "  -v, --verbose             Verbose output\n"
              << "  -h, --help                Show this help message\n\n"
              << "Examples:\n"
              << "  " << program_name << " words.txt -o output.json\n"
              << "  " << program_name << " greek_names.txt -o greek.json --markov-order 3 --enable-syllables\n";
}

std::optional<Config> parse_arguments(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return std::nullopt;
    }

    Config config;
    bool has_input = false;
    bool has_output = false;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return std::nullopt;
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " requires an argument\n";
                return std::nullopt;
            }
            config.output_file = argv[++i];
            has_output = true;
        }
        else if (arg == "--markov-order") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --markov-order requires an argument\n";
                return std::nullopt;
            }
            try {
                int order = std::stoi(argv[++i]);
                if (order < 1 || order > 3) {
                    std::cerr << "Error: Markov order must be between 1 and 3\n";
                    return std::nullopt;
                }
                config.markov_order = order;
            } catch (...) {
                std::cerr << "Error: Invalid markov order value\n";
                return std::nullopt;
            }
        }
        else if (arg == "--enable-syllables") {
            config.enable_syllables = true;
        }
        else if (arg == "--enable-components") {
            config.enable_components = true;
        }
        else if (arg == "--min-length") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --min-length requires an argument\n";
                return std::nullopt;
            }
            try {
                int len = std::stoi(argv[++i]);
                if (len < 1) {
                    std::cerr << "Error: Minimum length must be at least 1\n";
                    return std::nullopt;
                }
                config.min_word_length = len;
            } catch (...) {
                std::cerr << "Error: Invalid min-length value\n";
                return std::nullopt;
            }
        }
        else if (arg == "-v" || arg == "--verbose") {
            config.verbose = true;
        }
        else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            return std::nullopt;
        }
        else {
            // Assume it's the input file
            if (!has_input) {
                config.input_file = std::string(arg);
                has_input = true;
            } else {
                std::cerr << "Error: Multiple input files specified\n";
                return std::nullopt;
            }
        }
    }

    if (!has_input) {
        std::cerr << "Error: No input file specified\n";
        print_usage(argv[0]);
        return std::nullopt;
    }

    if (!has_output) {
        std::cerr << "Error: No output file specified (use -o or --output)\n";
        print_usage(argv[0]);
        return std::nullopt;
    }

    return config;
}

} // namespace nameanalyzer
