#include "cli_parser.hpp"
#include "word_reader.hpp"
#include "ngram_extractor.hpp"
#include "syllable_detector.hpp"
#include "component_extractor.hpp"
#include "json_writer.hpp"
#include <iostream>
#include <stdexcept>

using namespace nameanalyzer;

int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        auto config_opt = parse_arguments(argc, argv);
        if (!config_opt) {
            return 1; // Help was shown or parsing failed
        }
        Config config = *config_opt;

        if (config.verbose) {
            std::cout << "NameAnalyzer - Word Pattern Analysis\n";
            std::cout << "=====================================\n";
            std::cout << "Input file: " << config.input_file << "\n";
            std::cout << "Output file: " << config.output_file << "\n";
            std::cout << "Markov order: " << config.markov_order << "\n";
            std::cout << "Syllable analysis: " << (config.enable_syllables ? "enabled" : "disabled") << "\n";
            std::cout << "Component analysis: " << (config.enable_components ? "enabled" : "disabled") << "\n";
            std::cout << "\n";
        }

        // Read words from input file
        if (config.verbose) {
            std::cout << "Reading words from file...\n";
        }
        auto words = read_words(config.input_file, config.min_word_length);
        if (config.verbose) {
            std::cout << "Loaded " << words.size() << " words\n\n";
        }

        // Initialize results structure
        AnalysisResults results;
        results.config = config;

        // Calculate basic statistics
        if (config.verbose) {
            std::cout << "Calculating statistics...\n";
        }
        results.stats.total_words = words.size();
        for (const auto& word : words) {
            results.stats.total_characters += word.length();
            results.stats.length_distribution[word.length()]++;
        }
        results.stats.avg_word_length = static_cast<double>(results.stats.total_characters) /
                                        static_cast<double>(results.stats.total_words);

        // Letter-level analysis
        if (config.verbose) {
            std::cout << "Analyzing letter patterns and building Markov chains...\n";
        }
        results.letter_analysis = analyze_letters(words, config.markov_order);

        // Syllable analysis (if enabled)
        if (config.enable_syllables) {
            if (config.verbose) {
                std::cout << "Detecting syllables...\n";
            }
            results.syllable_analysis = analyze_syllables(words, config.markov_order);

            // Count total syllables
            for (const auto& [syll, count] : results.syllable_analysis.syllable_frequencies) {
                results.stats.total_syllables += count;
            }
            results.stats.avg_syllables_per_word =
                static_cast<double>(results.stats.total_syllables) /
                static_cast<double>(results.stats.total_words);

            if (config.verbose) {
                std::cout << "Found " << results.syllable_analysis.all_syllables.size()
                          << " unique syllables\n";
            }
        }

        // Component analysis (if enabled)
        if (config.enable_components) {
            if (config.verbose) {
                std::cout << "Extracting onset/nucleus/coda components...\n";
            }
            results.component_analysis = analyze_components(words);

            if (config.verbose) {
                std::cout << "Found " << results.component_analysis.frequencies.onsets.size()
                          << " unique onsets, "
                          << results.component_analysis.frequencies.nuclei.size()
                          << " unique nuclei, "
                          << results.component_analysis.frequencies.codas.size()
                          << " unique codas\n";
            }
        }

        // Write JSON output
        if (config.verbose) {
            std::cout << "\nWriting results to " << config.output_file << "...\n";
        }
        write_json_output(results, config.output_file);

        if (config.verbose) {
            std::cout << "Done!\n";
        } else {
            std::cout << "Analysis complete. Output written to " << config.output_file << "\n";
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
