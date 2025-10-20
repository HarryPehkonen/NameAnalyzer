#include "ngram_extractor.hpp"
#include "markov_builder.hpp"
#include <string_view>

namespace nameanalyzer {

void extract_ngrams(std::string_view word, int n, FrequencyMap& ngrams) {
    if (static_cast<int>(word.length()) < n) {
        return;
    }

    for (std::size_t i = 0; i <= word.length() - n; ++i) {
        std::string ngram(word.substr(i, n));
        ngrams[ngram]++;
    }
}

void extract_positional_ngrams(std::string_view word, int n, PositionalFrequencies& pos_freq) {
    if (static_cast<int>(word.length()) < n) {
        return;
    }

    // Start: first n-gram
    if (word.length() >= static_cast<std::size_t>(n)) {
        std::string start_ngram(word.substr(0, n));
        pos_freq.start[start_ngram]++;
    }

    // End: last n-gram
    if (word.length() >= static_cast<std::size_t>(n)) {
        std::string end_ngram(word.substr(word.length() - n, n));
        pos_freq.end[end_ngram]++;
    }

    // Middle: all n-grams except first and last
    if (word.length() > static_cast<std::size_t>(n)) {
        for (std::size_t i = 1; i < word.length() - n; ++i) {
            std::string mid_ngram(word.substr(i, n));
            pos_freq.middle[mid_ngram]++;
        }
    }
}

LetterAnalysis analyze_letters(const std::vector<std::string>& words, int markov_order) {
    LetterAnalysis analysis;

    for (const auto& word : words) {
        // Extract n-grams of various sizes
        extract_ngrams(word, 1, analysis.unigrams);
        extract_ngrams(word, 2, analysis.bigrams);
        extract_ngrams(word, 3, analysis.trigrams);
        extract_ngrams(word, 4, analysis.fourgrams);

        // Extract positional n-grams
        extract_positional_ngrams(word, 2, analysis.positional_bigrams);
        extract_positional_ngrams(word, 3, analysis.positional_trigrams);
    }

    // Build Markov chains for requested orders (1 to markov_order)
    for (int order = 1; order <= markov_order; ++order) {
        analysis.markov_chains[order] = build_markov_chain(words, order);
    }

    return analysis;
}

} // namespace nameanalyzer
