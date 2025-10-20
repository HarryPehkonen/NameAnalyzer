#pragma once

#include "types.hpp"
#include <vector>
#include <string>
#include <string_view>

namespace nameanalyzer {

/// Detect if a character is a vowel (including 'y' in certain contexts)
bool is_vowel(char c);

/// Detect if a character is a consonant
bool is_consonant(char c);

/// Split a word into syllables using heuristic rules
std::vector<Syllable> detect_syllables(std::string_view word);

/// Analyze syllables from word corpus
SyllableAnalysis analyze_syllables(const std::vector<std::string>& words, int markov_order);

} // namespace nameanalyzer
