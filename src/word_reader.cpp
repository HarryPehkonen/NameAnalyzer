#include "word_reader.hpp"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <utf8proc.h>

namespace nameanalyzer {

std::string to_lowercase(std::string_view str) {
    // Use utf8proc for proper Unicode case folding
    utf8proc_uint8_t* result = nullptr;
    utf8proc_ssize_t result_size = utf8proc_map(
        reinterpret_cast<const utf8proc_uint8_t*>(str.data()),
        static_cast<utf8proc_ssize_t>(str.size()),
        &result,
        static_cast<utf8proc_option_t>(UTF8PROC_NULLTERM | UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_CASEFOLD)
    );

    if (result_size < 0 || !result) {
        throw std::runtime_error("UTF-8 case conversion failed for: " + std::string(str));
    }

    std::string output(reinterpret_cast<const char*>(result), static_cast<size_t>(result_size));
    free(result);  // utf8proc uses malloc
    return output;
}

std::vector<std::string> read_words(std::string_view filename, int min_length) {
    std::ifstream file(filename.data());
    if (!file) {
        throw std::runtime_error("Failed to open file: " + std::string(filename));
    }

    std::vector<std::string> words;
    std::string line;

    while (std::getline(file, line)) {

	// Remove any comments
	auto comment_pos = line.find('#');
	if (comment_pos != std::string::npos) {
	    line.erase(comment_pos);
	}

        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.empty()) {
            continue;
        }

        // Convert to lowercase
        std::string word = to_lowercase(line);

        // Filter by minimum length
        if (static_cast<int>(word.length()) >= min_length) {
            words.push_back(std::move(word));
        }
    }

    if (words.empty()) {
        throw std::runtime_error("No valid words found in file");
    }

    return words;
}

} // namespace nameanalyzer
