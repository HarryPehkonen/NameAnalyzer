#include "markov_builder.hpp"
#include <utf8proc.h>
#include <vector>

namespace nameanalyzer {

// Helper to analyze UTF-8 codepoints
struct Utf8StringInfo {
    std::vector<std::size_t> byte_positions;
    std::size_t num_codepoints;
};

Utf8StringInfo analyze_utf8_string(const std::string& str) {
    Utf8StringInfo info;
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
            byte_pos++;
            continue;
        }

        byte_pos += static_cast<std::size_t>(bytes_read);
        info.byte_positions.push_back(byte_pos);
    }

    info.num_codepoints = info.byte_positions.size() - 1;
    return info;
}

MarkovChain build_markov_chain(const std::vector<std::string>& words, int order) {
    MarkovChain chain;

    for (const auto& word : words) {
        // Add start marker for beginning of word
        std::string augmented = std::string(order, '^') + word + "$";

        Utf8StringInfo utf8_info = analyze_utf8_string(augmented);

        // Build Markov chain using codepoint positions
        if (utf8_info.num_codepoints <= static_cast<std::size_t>(order)) {
            continue;  // Not enough characters
        }

        for (std::size_t i = 0; i < utf8_info.num_codepoints - static_cast<std::size_t>(order); ++i) {
            // Extract context (order characters)
            std::size_t context_start_byte = utf8_info.byte_positions[i];
            std::size_t context_end_byte = utf8_info.byte_positions[i + order];
            std::string context = augmented.substr(context_start_byte, context_end_byte - context_start_byte);

            // Extract next character
            std::size_t next_start_byte = utf8_info.byte_positions[i + order];
            std::size_t next_end_byte = utf8_info.byte_positions[i + order + 1];
            std::string next_char = augmented.substr(next_start_byte, next_end_byte - next_start_byte);

            chain[context][next_char]++;
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
