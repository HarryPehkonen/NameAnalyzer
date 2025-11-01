#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utf8proc.h>
#include "word_reader.hpp"

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
    std::string blacklist_chars = "(),.!@$%^&*-_=+[{]}/?<>";

    while (std::getline(file, line)) {

	// Remove any comments
	auto comment_pos = line.find('#');
	if (comment_pos != std::string::npos) {
	    line.erase(comment_pos);
	}

	// split by whitespace
	std::istringstream iss(to_lowercase(line));
	std::string word;
	while (iss >> word) {

	    // ensure minimum length
	    if (static_cast<int>(word.length()) < min_length) {
		continue;
	    }

	    // ensure no bad characters
	    if (word.find_first_of(blacklist_chars) != std::string::npos) {
		std::cout << "Skipping " << word << "\n";
		continue;
	    }

	    words.push_back(std::move(word));
	}
    }

    if (words.empty()) {
        throw std::runtime_error("No valid words found in file");
    }

    return words;
}

} // namespace nameanalyzer
