#include "word_reader.hpp"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace nameanalyzer {

std::string to_lowercase(std::string_view str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        result.push_back(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
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
