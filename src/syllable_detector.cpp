#include "syllable_detector.hpp"
#include "markov_builder.hpp"
#include <algorithm>
#include <cctype>
#include <utf8proc.h>

namespace nameanalyzer {

// Helper to check if a UTF-8 codepoint is a vowel
bool is_vowel_codepoint(utf8proc_int32_t cp) {
    // Basic Latin vowels (most common)
    if (cp == 'a' || cp == 'e' || cp == 'i' || cp == 'o' || cp == 'u' || cp == 'y' ||
        cp == 'A' || cp == 'E' || cp == 'I' || cp == 'O' || cp == 'U' || cp == 'Y') {
        return true;
    }

    // For non-ASCII, check if it's alphabetic but not a vowel
    // Since we've case-folded the input, we mainly care about lowercase
    // This is a simplified vowel detector - could be expanded with Unicode vowel ranges
    return false;
}

// Helper to convert codepoint positions to byte positions
struct CodepointInfo {
    std::vector<std::size_t> byte_positions;  // byte_positions[i] = byte offset of i-th codepoint
    std::size_t num_codepoints;
};

CodepointInfo analyze_codepoints(std::string_view str) {
    CodepointInfo info;
    info.byte_positions.push_back(0);  // First codepoint starts at byte 0

    std::size_t byte_pos = 0;
    while (byte_pos < str.size()) {
        utf8proc_int32_t codepoint;
        utf8proc_ssize_t bytes_read = utf8proc_iterate(
            reinterpret_cast<const utf8proc_uint8_t*>(str.data() + byte_pos),
            static_cast<utf8proc_ssize_t>(str.size() - byte_pos),
            &codepoint
        );

        if (bytes_read <= 0) {
            // Invalid UTF-8 or error - skip this byte
            byte_pos++;
            continue;
        }

        byte_pos += static_cast<std::size_t>(bytes_read);
        info.byte_positions.push_back(byte_pos);
    }

    info.num_codepoints = info.byte_positions.size() - 1;  // Subtract the final sentinel
    return info;
}

// Get codepoint at a given codepoint index
utf8proc_int32_t get_codepoint_at(std::string_view str, const CodepointInfo& info, std::size_t cp_idx) {
    if (cp_idx >= info.num_codepoints) {
        return -1;  // Out of bounds
    }

    std::size_t byte_pos = info.byte_positions[cp_idx];
    utf8proc_int32_t codepoint;
    utf8proc_iterate(
        reinterpret_cast<const utf8proc_uint8_t*>(str.data() + byte_pos),
        static_cast<utf8proc_ssize_t>(str.size() - byte_pos),
        &codepoint
    );

    return codepoint;
}

bool is_vowel(char c) {
    char lower = std::tolower(static_cast<unsigned char>(c));
    return lower == 'a' || lower == 'e' || lower == 'i' ||
           lower == 'o' || lower == 'u' || lower == 'y';
}

bool is_consonant(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) && !is_vowel(c);
}

std::vector<Syllable> detect_syllables(std::string_view word) {
    std::vector<Syllable> syllables;

    if (word.empty()) {
        return syllables;
    }

    // Analyze UTF-8 codepoints to get byte positions
    CodepointInfo cp_info = analyze_codepoints(word);

    // Find all vowel groups (nuclei) using CODEPOINT positions
    std::vector<std::pair<std::size_t, std::size_t>> vowel_groups; // codepoint start, end+1

    std::size_t i = 0;
    while (i < cp_info.num_codepoints) {
        utf8proc_int32_t cp = get_codepoint_at(word, cp_info, i);
        if (is_vowel_codepoint(cp)) {
            std::size_t start = i;
            // Collect consecutive vowels as a single nucleus
            while (i < cp_info.num_codepoints) {
                cp = get_codepoint_at(word, cp_info, i);
                if (!is_vowel_codepoint(cp)) {
                    break;
                }
                ++i;
            }
            vowel_groups.emplace_back(start, i);
        } else {
            ++i;
        }
    }

    // If no vowels found, treat whole word as one syllable with no nucleus
    if (vowel_groups.empty()) {
        Syllable syll;
        syll.onset = std::string(word);
        syllables.push_back(syll);
        return syllables;
    }

    // Split consonants between vowel groups
    for (std::size_t vg_idx = 0; vg_idx < vowel_groups.size(); ++vg_idx) {
        Syllable syll;

        auto [v_start_cp, v_end_cp] = vowel_groups[vg_idx];

        // Convert codepoint positions to byte positions
        std::size_t v_start_byte = cp_info.byte_positions[v_start_cp];
        std::size_t v_end_byte = cp_info.byte_positions[v_end_cp];

        syll.nucleus = std::string(word.substr(v_start_byte, v_end_byte - v_start_byte));

        // Determine onset (in codepoint positions)
        std::size_t onset_start_cp = (vg_idx == 0) ? 0 : vowel_groups[vg_idx - 1].second;
        std::size_t onset_end_cp = v_start_cp;

        // Determine coda (in codepoint positions)
        std::size_t coda_start_cp = v_end_cp;
        std::size_t coda_end_cp = (vg_idx + 1 < vowel_groups.size()) ?
                                   vowel_groups[vg_idx + 1].first : cp_info.num_codepoints;

        // Split consonants between syllables using heuristic rules
        if (vg_idx > 0 && onset_start_cp < onset_end_cp) {
            // Consonants between previous vowel and current vowel
            std::size_t num_consonants = onset_end_cp - onset_start_cp;

            if (num_consonants == 1) {
                // Single consonant goes to onset: V-CV
                std::size_t onset_byte = cp_info.byte_positions[onset_start_cp];
                std::size_t onset_byte_end = cp_info.byte_positions[onset_start_cp + 1];
                syll.onset = std::string(word.substr(onset_byte, onset_byte_end - onset_byte));
            } else if (num_consonants >= 2) {
                // Two or more consonants: split them
                // Give first consonant to previous syllable's coda, rest to current onset

                // Update previous syllable's coda
                if (!syllables.empty()) {
                    std::size_t coda_byte = cp_info.byte_positions[onset_start_cp];
                    std::size_t coda_byte_end = cp_info.byte_positions[onset_start_cp + 1];
                    syllables.back().coda = std::string(word.substr(coda_byte, coda_byte_end - coda_byte));
                }

                // Rest goes to current onset
                if (num_consonants > 1) {
                    std::size_t onset_byte = cp_info.byte_positions[onset_start_cp + 1];
                    std::size_t onset_byte_end = cp_info.byte_positions[onset_end_cp];
                    syll.onset = std::string(word.substr(onset_byte, onset_byte_end - onset_byte));
                }
            }
        } else if (vg_idx == 0) {
            // First syllable: all initial consonants are onset
            std::size_t onset_byte = cp_info.byte_positions[onset_start_cp];
            std::size_t onset_byte_end = cp_info.byte_positions[onset_end_cp];
            syll.onset = std::string(word.substr(onset_byte, onset_byte_end - onset_byte));
        }

        // Handle coda for last syllable
        if (vg_idx == vowel_groups.size() - 1) {
            std::size_t coda_byte = cp_info.byte_positions[coda_start_cp];
            std::size_t coda_byte_end = cp_info.byte_positions[coda_end_cp];
            syll.coda = std::string(word.substr(coda_byte, coda_byte_end - coda_byte));
        }

        syllables.push_back(syll);
    }

    return syllables;
}

SyllableAnalysis analyze_syllables(const std::vector<std::string>& words, int markov_order) {
    SyllableAnalysis analysis;
    std::vector<std::string> all_syllables_flat; // For Markov chain building

    for (const auto& word : words) {
        auto syllables = detect_syllables(word);

        for (std::size_t i = 0; i < syllables.size(); ++i) {
            const auto& syll = syllables[i];
            std::string syll_str = syll.to_string();

            // Collect unique syllables
            if (std::find(analysis.all_syllables.begin(),
                         analysis.all_syllables.end(),
                         syll_str) == analysis.all_syllables.end()) {
                analysis.all_syllables.push_back(syll_str);
            }

            // Count frequencies
            analysis.syllable_frequencies[syll_str]++;

            // Positional frequencies
            if (i == 0) {
                analysis.positional_syllables.start[syll_str]++;
            } else if (i == syllables.size() - 1) {
                analysis.positional_syllables.end[syll_str]++;
            } else {
                analysis.positional_syllables.middle[syll_str]++;
            }

            // For Markov chain building
            all_syllables_flat.push_back(syll_str);
        }
    }

    // Build syllable-level Markov chains
    for (int order = 1; order <= markov_order; ++order) {
        analysis.syllable_markov[order] = build_syllable_markov_chain(all_syllables_flat, order);
    }

    return analysis;
}

} // namespace nameanalyzer
