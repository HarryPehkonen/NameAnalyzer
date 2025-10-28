#include "ngram_extractor.hpp"
#include "markov_builder.hpp"
#include <string_view>
#include <utf8proc.h>
#include <vector>

namespace nameanalyzer {

// Helper to get UTF-8 codepoint count and byte positions
struct Utf8Info {
    std::vector<std::size_t> byte_positions;  // byte_positions[i] = byte offset of i-th codepoint
    std::size_t num_codepoints;
};

Utf8Info analyze_utf8(std::string_view str) {
    Utf8Info info;
    info.byte_positions.push_back(0);

    std::size_t byte_pos = 0;
    while (byte_pos < str.size()) {
        utf8proc_int32_t codepoint;
        utf8proc_ssize_t bytes_read = utf8proc_iterate(
            reinterpret_cast<const utf8proc_uint8_t*>(str.data() + byte_pos),
            static_cast<utf8proc_ssize_t>(str.size() - byte_pos),
            &codepoint
        );

        if (bytes_read <= 0) {
            byte_pos++;  // Skip invalid byte
            continue;
        }

        byte_pos += static_cast<std::size_t>(bytes_read);
        info.byte_positions.push_back(byte_pos);
    }

    info.num_codepoints = info.byte_positions.size() - 1;
    return info;
}

void extract_ngrams(std::string_view word, int n, FrequencyMap& ngrams) {
    Utf8Info utf8_info = analyze_utf8(word);

    if (static_cast<int>(utf8_info.num_codepoints) < n) {
        return;
    }

    // Extract n-grams using codepoint positions
    for (std::size_t i = 0; i <= utf8_info.num_codepoints - static_cast<std::size_t>(n); ++i) {
        std::size_t start_byte = utf8_info.byte_positions[i];
        std::size_t end_byte = utf8_info.byte_positions[i + n];
        std::string ngram(word.substr(start_byte, end_byte - start_byte));
        ngrams[ngram]++;
    }
}

void extract_positional_ngrams(std::string_view word, int n, PositionalFrequencies& pos_freq) {
    Utf8Info utf8_info = analyze_utf8(word);

    if (static_cast<int>(utf8_info.num_codepoints) < n) {
        return;
    }

    // Start: first n-gram
    {
        std::size_t start_byte = utf8_info.byte_positions[0];
        std::size_t end_byte = utf8_info.byte_positions[n];
        std::string start_ngram(word.substr(start_byte, end_byte - start_byte));
        pos_freq.start[start_ngram]++;
    }

    // End: last n-gram
    {
        std::size_t start_idx = utf8_info.num_codepoints - static_cast<std::size_t>(n);
        std::size_t start_byte = utf8_info.byte_positions[start_idx];
        std::size_t end_byte = utf8_info.byte_positions[utf8_info.num_codepoints];
        std::string end_ngram(word.substr(start_byte, end_byte - start_byte));
        pos_freq.end[end_ngram]++;
    }

    // Middle: all n-grams except first and last
    if (utf8_info.num_codepoints > static_cast<std::size_t>(n)) {
        for (std::size_t i = 1; i < utf8_info.num_codepoints - static_cast<std::size_t>(n); ++i) {
            std::size_t start_byte = utf8_info.byte_positions[i];
            std::size_t end_byte = utf8_info.byte_positions[i + n];
            std::string mid_ngram(word.substr(start_byte, end_byte - start_byte));
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
