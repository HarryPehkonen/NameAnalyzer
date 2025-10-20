#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace nameanalyzer {

/// Configuration options from CLI
struct Config {
    std::string input_file;
    std::string output_file;
    int markov_order = 2;           // Default to 2nd order
    bool enable_syllables = false;
    bool enable_components = false;
    int min_word_length = 2;        // Ignore very short words
    bool verbose = false;
};

/// Position in word for position-aware analysis
enum class Position {
    Start,
    Middle,
    End
};

/// Frequency map for n-grams or syllables
using FrequencyMap = std::map<std::string, std::size_t>;

/// Position-aware frequency maps
struct PositionalFrequencies {
    FrequencyMap start;
    FrequencyMap middle;
    FrequencyMap end;
};

/// Markov chain: given context (previous n chars/syllables), what comes next?
/// Maps context -> {next_item -> frequency}
using MarkovChain = std::map<std::string, FrequencyMap>;

/// Syllable structure (onset-nucleus-coda)
struct Syllable {
    std::string onset;   // Initial consonant cluster (can be empty)
    std::string nucleus; // Vowel group (required)
    std::string coda;    // Final consonant cluster (can be empty)

    std::string to_string() const {
        return onset + nucleus + coda;
    }
};

/// Component frequencies (for syllable assembly)
struct ComponentFrequencies {
    FrequencyMap onsets;
    FrequencyMap nuclei;
    FrequencyMap codas;
};

/// Letter-level analysis results
struct LetterAnalysis {
    FrequencyMap unigrams;          // Single characters
    FrequencyMap bigrams;           // 2-character sequences
    FrequencyMap trigrams;          // 3-character sequences
    FrequencyMap fourgrams;         // 4-character sequences

    PositionalFrequencies positional_bigrams;
    PositionalFrequencies positional_trigrams;

    std::map<int, MarkovChain> markov_chains; // order -> chain
};

/// Syllable-level analysis results
struct SyllableAnalysis {
    std::vector<std::string> all_syllables;  // Unique syllables found
    FrequencyMap syllable_frequencies;
    PositionalFrequencies positional_syllables;
    std::map<int, MarkovChain> syllable_markov; // order -> chain
};

/// Component-level analysis results
struct ComponentAnalysis {
    ComponentFrequencies frequencies;
    PositionalFrequencies positional_onsets;
    PositionalFrequencies positional_codas;
};

/// Overall statistics
struct CorpusStats {
    std::size_t total_words = 0;
    std::size_t total_characters = 0;
    std::size_t total_syllables = 0;
    double avg_word_length = 0.0;
    double avg_syllables_per_word = 0.0;
    std::map<std::size_t, std::size_t> length_distribution; // word_length -> count
};

/// Complete analysis results
struct AnalysisResults {
    Config config;
    CorpusStats stats;
    LetterAnalysis letter_analysis;
    SyllableAnalysis syllable_analysis;  // Only if enabled
    ComponentAnalysis component_analysis; // Only if enabled
};

} // namespace nameanalyzer
