#include "json_writer.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace nameanalyzer {

// Helper to escape JSON strings
static std::string json_escape(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

// Helper to write FrequencyMap as JSON object
static void write_frequency_map(std::ostream& out, const FrequencyMap& freq_map, const std::string& indent) {
    out << "{\n";
    bool first = true;
    for (const auto& [key, count] : freq_map) {
        if (!first) out << ",\n";
        first = false;
        out << indent << "  \"" << json_escape(key) << "\": " << count;
    }
    if (!freq_map.empty()) out << "\n";
    out << indent << "}";
}

// Helper to write PositionalFrequencies as JSON
static void write_positional_frequencies(std::ostream& out, const PositionalFrequencies& pos_freq, const std::string& indent) {
    out << "{\n";
    out << indent << "  \"start\": ";
    write_frequency_map(out, pos_freq.start, indent + "  ");
    out << ",\n";
    out << indent << "  \"middle\": ";
    write_frequency_map(out, pos_freq.middle, indent + "  ");
    out << ",\n";
    out << indent << "  \"end\": ";
    write_frequency_map(out, pos_freq.end, indent + "  ");
    out << "\n" << indent << "}";
}

// Helper to write MarkovChain as JSON
static void write_markov_chain(std::ostream& out, const MarkovChain& chain, const std::string& indent) {
    out << "{\n";
    bool first = true;
    for (const auto& [context, next_map] : chain) {
        if (!first) out << ",\n";
        first = false;
        out << indent << "  \"" << json_escape(context) << "\": ";
        write_frequency_map(out, next_map, indent + "  ");
    }
    if (!chain.empty()) out << "\n";
    out << indent << "}";
}

void write_json_output(const AnalysisResults& results, const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile) {
        throw std::runtime_error("Failed to open output file: " + filename);
    }

    outfile << "{\n";

    // Config section
    outfile << "  \"config\": {\n";
    outfile << "    \"input_file\": \"" << json_escape(results.config.input_file) << "\",\n";
    outfile << "    \"markov_order\": " << results.config.markov_order << ",\n";
    outfile << "    \"min_word_length\": " << results.config.min_word_length << ",\n";
    outfile << "    \"syllables_enabled\": " << (results.config.enable_syllables ? "true" : "false") << ",\n";
    outfile << "    \"components_enabled\": " << (results.config.enable_components ? "true" : "false") << "\n";
    outfile << "  },\n";

    // Stats section
    outfile << "  \"stats\": {\n";
    outfile << "    \"total_words\": " << results.stats.total_words << ",\n";
    outfile << "    \"total_characters\": " << results.stats.total_characters << ",\n";
    outfile << "    \"total_syllables\": " << results.stats.total_syllables << ",\n";
    outfile << "    \"avg_word_length\": " << results.stats.avg_word_length << ",\n";
    outfile << "    \"avg_syllables_per_word\": " << results.stats.avg_syllables_per_word << ",\n";
    outfile << "    \"length_distribution\": {\n";

    bool first = true;
    for (const auto& [len, count] : results.stats.length_distribution) {
        if (!first) outfile << ",\n";
        first = false;
        outfile << "      \"" << len << "\": " << count;
    }
    if (!results.stats.length_distribution.empty()) outfile << "\n";
    outfile << "    }\n";
    outfile << "  },\n";

    // Letter analysis
    outfile << "  \"letter_analysis\": {\n";
    outfile << "    \"unigrams\": ";
    write_frequency_map(outfile, results.letter_analysis.unigrams, "    ");
    outfile << ",\n    \"bigrams\": ";
    write_frequency_map(outfile, results.letter_analysis.bigrams, "    ");
    outfile << ",\n    \"trigrams\": ";
    write_frequency_map(outfile, results.letter_analysis.trigrams, "    ");
    outfile << ",\n    \"fourgrams\": ";
    write_frequency_map(outfile, results.letter_analysis.fourgrams, "    ");
    outfile << ",\n    \"positional_bigrams\": ";
    write_positional_frequencies(outfile, results.letter_analysis.positional_bigrams, "    ");
    outfile << ",\n    \"positional_trigrams\": ";
    write_positional_frequencies(outfile, results.letter_analysis.positional_trigrams, "    ");
    outfile << ",\n    \"markov_chains\": {\n";

    first = true;
    for (const auto& [order, chain] : results.letter_analysis.markov_chains) {
        if (!first) outfile << ",\n";
        first = false;
        outfile << "      \"order_" << order << "\": ";
        write_markov_chain(outfile, chain, "      ");
    }
    if (!results.letter_analysis.markov_chains.empty()) outfile << "\n";
    outfile << "    }\n";
    outfile << "  }";

    // Syllable analysis (if enabled)
    if (results.config.enable_syllables) {
        outfile << ",\n  \"syllable_analysis\": {\n";
        outfile << "    \"all_syllables\": [";
        first = true;
        for (const auto& syll : results.syllable_analysis.all_syllables) {
            if (!first) outfile << ", ";
            first = false;
            outfile << "\"" << json_escape(syll) << "\"";
        }
        outfile << "],\n";
        outfile << "    \"syllable_frequencies\": ";
        write_frequency_map(outfile, results.syllable_analysis.syllable_frequencies, "    ");
        outfile << ",\n    \"positional_syllables\": ";
        write_positional_frequencies(outfile, results.syllable_analysis.positional_syllables, "    ");
        outfile << ",\n    \"syllable_markov\": {\n";

        first = true;
        for (const auto& [order, chain] : results.syllable_analysis.syllable_markov) {
            if (!first) outfile << ",\n";
            first = false;
            outfile << "      \"order_" << order << "\": ";
            write_markov_chain(outfile, chain, "      ");
        }
        if (!results.syllable_analysis.syllable_markov.empty()) outfile << "\n";
        outfile << "    }\n";
        outfile << "  }";
    }

    // Component analysis (if enabled)
    if (results.config.enable_components) {
        outfile << ",\n  \"component_analysis\": {\n";
        outfile << "    \"frequencies\": {\n";
        outfile << "      \"onsets\": ";
        write_frequency_map(outfile, results.component_analysis.frequencies.onsets, "      ");
        outfile << ",\n      \"nuclei\": ";
        write_frequency_map(outfile, results.component_analysis.frequencies.nuclei, "      ");
        outfile << ",\n      \"codas\": ";
        write_frequency_map(outfile, results.component_analysis.frequencies.codas, "      ");
        outfile << "\n    },\n";
        outfile << "    \"positional_onsets\": ";
        write_positional_frequencies(outfile, results.component_analysis.positional_onsets, "    ");
        outfile << ",\n    \"positional_codas\": ";
        write_positional_frequencies(outfile, results.component_analysis.positional_codas, "    ");
        outfile << "\n  }";
    }

    outfile << "\n}\n";
}

} // namespace nameanalyzer
