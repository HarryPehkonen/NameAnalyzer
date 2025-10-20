#include "syllable_detector.hpp"
#include "markov_builder.hpp"
#include <algorithm>
#include <cctype>

namespace nameanalyzer {

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

    // Find all vowel groups (nuclei)
    std::vector<std::pair<std::size_t, std::size_t>> vowel_groups; // start, end+1

    std::size_t i = 0;
    while (i < word.length()) {
        if (is_vowel(word[i])) {
            std::size_t start = i;
            // Collect consecutive vowels as a single nucleus
            while (i < word.length() && is_vowel(word[i])) {
                ++i;
            }
            vowel_groups.emplace_back(start, i);
        } else {
            ++i;
        }
    }

    // If no vowels found, treat whole word as one syllable with no nucleus
    // (This shouldn't happen with real words, but handle edge cases)
    if (vowel_groups.empty()) {
        Syllable syll;
        syll.onset = std::string(word);
        syllables.push_back(syll);
        return syllables;
    }

    // Split consonants between vowel groups
    for (std::size_t vg_idx = 0; vg_idx < vowel_groups.size(); ++vg_idx) {
        Syllable syll;

        auto [v_start, v_end] = vowel_groups[vg_idx];
        syll.nucleus = std::string(word.substr(v_start, v_end - v_start));

        // Determine onset
        std::size_t onset_start = (vg_idx == 0) ? 0 : vowel_groups[vg_idx - 1].second;
        std::size_t onset_end = v_start;

        // Determine coda
        std::size_t coda_start = v_end;
        std::size_t coda_end = (vg_idx + 1 < vowel_groups.size()) ?
                                vowel_groups[vg_idx + 1].first : word.length();

        // Split consonants between syllables using heuristic rules
        if (vg_idx > 0 && onset_start < onset_end) {
            // Consonants between previous vowel and current vowel
            std::size_t num_consonants = onset_end - onset_start;

            if (num_consonants == 1) {
                // Single consonant goes to onset: V-CV
                syll.onset = std::string(word.substr(onset_start, num_consonants));
                // Previous syllable's coda is empty (already set)
            } else if (num_consonants >= 2) {
                // Two or more consonants: split them
                // Generally: VC-CV or VC-CCV for clusters

                // Simple heuristic: give first consonant to previous syllable's coda,
                // rest to current onset
                // This handles VC-CV pattern

                // Update previous syllable's coda
                if (!syllables.empty()) {
                    syllables.back().coda = std::string(word.substr(onset_start, 1));
                }

                // Rest goes to current onset
                if (num_consonants > 1) {
                    syll.onset = std::string(word.substr(onset_start + 1, num_consonants - 1));
                }
            }
        } else if (vg_idx == 0) {
            // First syllable: all initial consonants are onset
            syll.onset = std::string(word.substr(onset_start, onset_end - onset_start));
        }

        // Handle coda for last syllable
        if (vg_idx == vowel_groups.size() - 1) {
            syll.coda = std::string(word.substr(coda_start, coda_end - coda_start));
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
