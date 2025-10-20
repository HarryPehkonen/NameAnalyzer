#pragma once

#include "types.hpp"
#include <vector>
#include <string>

namespace nameanalyzer {

/// Extract letter-level n-grams and statistics from word corpus
LetterAnalysis analyze_letters(const std::vector<std::string>& words, int markov_order);

/// Extract n-grams of specific size from a word
void extract_ngrams(std::string_view word, int n, FrequencyMap& ngrams);

/// Extract positional n-grams (start, middle, end)
void extract_positional_ngrams(std::string_view word, int n, PositionalFrequencies& pos_freq);

} // namespace nameanalyzer
