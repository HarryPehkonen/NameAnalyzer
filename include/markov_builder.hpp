#pragma once

#include "types.hpp"
#include <vector>
#include <string>

namespace nameanalyzer {

/// Build a Markov chain of given order from words
/// Order = number of previous characters to consider as context
MarkovChain build_markov_chain(const std::vector<std::string>& words, int order);

/// Build a Markov chain for syllables
MarkovChain build_syllable_markov_chain(const std::vector<std::string>& syllables, int order);

} // namespace nameanalyzer
