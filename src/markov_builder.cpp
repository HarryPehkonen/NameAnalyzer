#include "markov_builder.hpp"

namespace nameanalyzer {

MarkovChain build_markov_chain(const std::vector<std::string>& words, int order) {
    MarkovChain chain;

    for (const auto& word : words) {
        // Add start marker for beginning of word
        std::string augmented = std::string(order, '^') + word + "$";

        for (std::size_t i = 0; i < augmented.length() - order; ++i) {
            std::string context = augmented.substr(i, order);
            char next_char = augmented[i + order];
            chain[context][std::string(1, next_char)]++;
        }
    }

    return chain;
}

MarkovChain build_syllable_markov_chain(const std::vector<std::string>& syllables, int order) {
    // For syllable-level Markov chains, we treat each syllable as a token
    // This is used when we have a sequence of syllables (from syllable detection)

    MarkovChain chain;

    if (syllables.size() <= static_cast<std::size_t>(order)) {
        return chain; // Not enough syllables
    }

    for (std::size_t i = 0; i < syllables.size() - order; ++i) {
        // Build context from 'order' syllables
        std::string context;
        for (int j = 0; j < order; ++j) {
            if (j > 0) context += "|";
            context += syllables[i + j];
        }

        // Next syllable
        const std::string& next = syllables[i + order];
        chain[context][next]++;
    }

    return chain;
}

} // namespace nameanalyzer
