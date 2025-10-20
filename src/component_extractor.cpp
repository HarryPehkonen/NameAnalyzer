#include "component_extractor.hpp"
#include "syllable_detector.hpp"

namespace nameanalyzer {

ComponentAnalysis analyze_components(const std::vector<std::string>& words) {
    ComponentAnalysis analysis;

    for (const auto& word : words) {
        auto syllables = detect_syllables(word);

        for (std::size_t i = 0; i < syllables.size(); ++i) {
            const auto& syll = syllables[i];

            // Count component frequencies
            analysis.frequencies.onsets[syll.onset]++;
            analysis.frequencies.nuclei[syll.nucleus]++;
            analysis.frequencies.codas[syll.coda]++;

            // Positional onset frequencies
            if (i == 0) {
                analysis.positional_onsets.start[syll.onset]++;
            } else if (i == syllables.size() - 1) {
                analysis.positional_onsets.end[syll.onset]++;
            } else {
                analysis.positional_onsets.middle[syll.onset]++;
            }

            // Positional coda frequencies
            if (i == 0) {
                analysis.positional_codas.start[syll.coda]++;
            } else if (i == syllables.size() - 1) {
                analysis.positional_codas.end[syll.coda]++;
            } else {
                analysis.positional_codas.middle[syll.coda]++;
            }
        }
    }

    return analysis;
}

} // namespace nameanalyzer
