#include "json_writer.hpp"
#include <jsom/json_document.hpp>
#include <fstream>

using namespace jsom;

namespace nameanalyzer {

// Helper to convert FrequencyMap to JsonDocument
static JsonDocument frequency_map_to_json(const FrequencyMap& freq_map) {
    std::map<std::string, JsonDocument> obj;
    for (const auto& [key, count] : freq_map) {
        obj[key] = JsonDocument(static_cast<int>(count));
    }
    return JsonDocument(obj);
}

// Helper to convert PositionalFrequencies to JsonDocument
static JsonDocument positional_frequencies_to_json(const PositionalFrequencies& pos_freq) {
    return JsonDocument{
        {"start", frequency_map_to_json(pos_freq.start)},
        {"middle", frequency_map_to_json(pos_freq.middle)},
        {"end", frequency_map_to_json(pos_freq.end)}
    };
}

// Helper to convert MarkovChain to JsonDocument
static JsonDocument markov_chain_to_json(const MarkovChain& chain) {
    std::map<std::string, JsonDocument> obj;
    for (const auto& [context, next_map] : chain) {
        obj[context] = frequency_map_to_json(next_map);
    }
    return JsonDocument(obj);
}

void write_json_output(const AnalysisResults& results, const std::string& filename) {
    // Config section
    JsonDocument config{
        {"input_file", JsonDocument(results.config.input_file)},
        {"markov_order", JsonDocument(results.config.markov_order)},
        {"min_word_length", JsonDocument(results.config.min_word_length)},
        {"syllables_enabled", JsonDocument(results.config.enable_syllables)},
        {"components_enabled", JsonDocument(results.config.enable_components)}
    };

    // Stats section
    std::map<std::string, JsonDocument> length_dist;
    for (const auto& [len, count] : results.stats.length_distribution) {
        length_dist[std::to_string(len)] = JsonDocument(static_cast<int>(count));
    }

    JsonDocument stats{
        {"total_words", JsonDocument(static_cast<int>(results.stats.total_words))},
        {"total_characters", JsonDocument(static_cast<int>(results.stats.total_characters))},
        {"total_syllables", JsonDocument(static_cast<int>(results.stats.total_syllables))},
        {"avg_word_length", JsonDocument(results.stats.avg_word_length)},
        {"avg_syllables_per_word", JsonDocument(results.stats.avg_syllables_per_word)},
        {"length_distribution", JsonDocument(length_dist)}
    };

    // Letter analysis section
    std::map<std::string, JsonDocument> markov_chains;
    for (const auto& [order, chain] : results.letter_analysis.markov_chains) {
        markov_chains["order_" + std::to_string(order)] = markov_chain_to_json(chain);
    }

    JsonDocument letter_analysis{
        {"unigrams", frequency_map_to_json(results.letter_analysis.unigrams)},
        {"bigrams", frequency_map_to_json(results.letter_analysis.bigrams)},
        {"trigrams", frequency_map_to_json(results.letter_analysis.trigrams)},
        {"fourgrams", frequency_map_to_json(results.letter_analysis.fourgrams)},
        {"positional_bigrams", positional_frequencies_to_json(results.letter_analysis.positional_bigrams)},
        {"positional_trigrams", positional_frequencies_to_json(results.letter_analysis.positional_trigrams)},
        {"markov_chains", JsonDocument(markov_chains)}
    };

    // Build root document
    std::map<std::string, JsonDocument> root_map;
    root_map["config"] = config;
    root_map["stats"] = stats;
    root_map["letter_analysis"] = letter_analysis;

    // Syllable analysis (if enabled)
    if (results.config.enable_syllables) {
        std::vector<JsonDocument> syllables_array;
        for (const auto& syll : results.syllable_analysis.all_syllables) {
            syllables_array.push_back(JsonDocument(syll));
        }

        std::map<std::string, JsonDocument> syllable_markov;
        for (const auto& [order, chain] : results.syllable_analysis.syllable_markov) {
            syllable_markov["order_" + std::to_string(order)] = markov_chain_to_json(chain);
        }

        root_map["syllable_analysis"] = JsonDocument{
            {"all_syllables", JsonDocument(syllables_array)},
            {"syllable_frequencies", frequency_map_to_json(results.syllable_analysis.syllable_frequencies)},
            {"positional_syllables", positional_frequencies_to_json(results.syllable_analysis.positional_syllables)},
            {"syllable_markov", JsonDocument(syllable_markov)}
        };
    }

    // Component analysis (if enabled)
    if (results.config.enable_components) {
        JsonDocument frequencies{
            {"onsets", frequency_map_to_json(results.component_analysis.frequencies.onsets)},
            {"nuclei", frequency_map_to_json(results.component_analysis.frequencies.nuclei)},
            {"codas", frequency_map_to_json(results.component_analysis.frequencies.codas)}
        };

        root_map["component_analysis"] = JsonDocument{
            {"frequencies", frequencies},
            {"positional_onsets", positional_frequencies_to_json(results.component_analysis.positional_onsets)},
            {"positional_codas", positional_frequencies_to_json(results.component_analysis.positional_codas)}
        };
    }

    // Create document and write to file
    JsonDocument doc(root_map);

    std::ofstream outfile(filename);
    if (!outfile) {
        throw std::runtime_error("Failed to open output file: " + filename);
    }

    outfile << doc.to_json(true); // true = pretty print
}

} // namespace nameanalyzer
