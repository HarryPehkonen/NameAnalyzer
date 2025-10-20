# NameAnalyzer

A C++20 tool for analyzing word patterns to extract statistical data for name generation. NameAnalyzer processes word lists to discover letter sequences, syllable structures, and phonetic patterns that can be used by NameGenerator to create pronounceable, authentic-sounding names.

## Purpose

Simple consonant-vowel patterns (like C-V-C-V) often produce unpronounceable results. NameAnalyzer takes a data-driven approach by analyzing real words to extract:

- **Letter-level patterns**: N-grams (1-4 letters) and their frequencies
- **Markov chains**: Probabilistic letter transitions for natural-looking sequences
- **Syllable structures**: Automatic syllable detection and frequency analysis
- **Phonetic components**: Onset/nucleus/coda patterns for syllable assembly

The output is a JSON file containing statistical patterns that NameGenerator can use to create names with different "flavors" (Greek, Norse, English, etc.) without hard-coded rules.

## Features

- ✅ Zero external dependencies - pure C++20
- ✅ Multi-level analysis (letters, syllables, components)
- ✅ Tunable Markov chain order (1st, 2nd, or 3rd order)
- ✅ Position-aware pattern extraction (start/middle/end of words)
- ✅ Heuristic syllable detection (~85-90% accuracy for English)
- ✅ Component-level breakdown (onset-nucleus-coda)
- ✅ Clean JSON output with full statistics
- ✅ UTF-8 text file input support

## Building

### Requirements
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.20+

### Quick Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The executable will be at `./build/nameanalyzer`

## Usage

### Basic Syntax
```bash
./build/nameanalyzer <input_file> -o <output_file> [options]
```

### Required Arguments
- `<input_file>` - Text file with one word per line (UTF-8 encoding)
- `-o, --output <file>` - Output JSON file for statistics

### Optional Arguments
- `--markov-order <1-3>` - Markov chain order (default: 2)
  - Order 1: Look at 1 previous letter
  - Order 2: Look at 2 previous letters (recommended)
  - Order 3: Look at 3 previous letters (more specific patterns)
- `--enable-syllables` - Enable syllable-level analysis
- `--enable-components` - Enable onset/nucleus/coda extraction
- `--min-length <n>` - Minimum word length to analyze (default: 2)
- `-v, --verbose` - Verbose output showing progress
- `-h, --help` - Show help message

## Examples

### Example 1: Basic Letter Analysis
Analyze letter patterns in English words:
```bash
./build/nameanalyzer english_words.txt -o english.json
```

### Example 2: Full Greek Mythology Analysis
Analyze Greek names with all features enabled:
```bash
./build/nameanalyzer greek_names.txt -o greek_profile.json \
  --markov-order 3 \
  --enable-syllables \
  --enable-components \
  -v
```

### Example 3: Quick Norse Names Analysis
Analyze Norse mythology names with 2nd-order Markov chains:
```bash
./build/nameanalyzer norse_names.txt -o norse.json \
  --markov-order 2 \
  --enable-syllables
```

### Example 4: Analyze Short Words/Prefixes
Include very short words in analysis:
```bash
./build/nameanalyzer prefixes.txt -o prefixes.json \
  --min-length 1
```

## Input File Format

Create a plain text file with one word per line:

```
strength
string
strong
thunder
throne
dragon
dream
athena
zeus
prometheus
hercules
```

**Notes:**
- Use UTF-8 encoding
- One word per line
- Empty lines are ignored
- Words are automatically converted to lowercase
- Whitespace is trimmed

## Output Format

NameAnalyzer generates a JSON file with the following structure:

```json
{
  "config": {
    "input_file": "...",
    "markov_order": 2,
    "min_word_length": 2,
    "syllables_enabled": true,
    "components_enabled": true
  },
  "stats": {
    "total_words": 15,
    "total_characters": 100,
    "total_syllables": 31,
    "avg_word_length": 6.67,
    "avg_syllables_per_word": 2.07,
    "length_distribution": { "4": 1, "5": 1, "6": 6, ... }
  },
  "letter_analysis": {
    "unigrams": { "a": 6, "e": 13, ... },
    "bigrams": { "th": 5, "st": 3, ... },
    "trigrams": { "str": 3, "thr": 2, ... },
    "fourgrams": { "stre": 1, ... },
    "positional_bigrams": {
      "start": { "st": 3, "th": 2, ... },
      "middle": { "re": 2, ... },
      "end": { "ng": 2, "on": 2, ... }
    },
    "positional_trigrams": { ... },
    "markov_chains": {
      "order_1": { "s": { "t": 5, "e": 2 }, ... },
      "order_2": { "st": { "r": 3 }, ... }
    }
  },
  "syllable_analysis": {
    "all_syllables": ["strength", "thun", "der", ...],
    "syllable_frequencies": { "strength": 1, ... },
    "positional_syllables": {
      "start": { "strength": 1, "thun": 1, ... },
      "middle": { ... },
      "end": { "der": 1, ... }
    },
    "syllable_markov": {
      "order_1": { "thun": { "der": 1 }, ... },
      "order_2": { ... }
    }
  },
  "component_analysis": {
    "frequencies": {
      "onsets": { "str": 3, "th": 1, "pr": 1, ... },
      "nuclei": { "e": 9, "o": 9, "a": 5, ... },
      "codas": { "ng": 2, "n": 3, ... }
    },
    "positional_onsets": {
      "start": { "str": 3, "th": 1, ... },
      "middle": { ... },
      "end": { ... }
    },
    "positional_codas": { ... }
  }
}
```

## Understanding the Output

### Letter Analysis
- **Unigrams**: Individual letter frequencies
- **Bigrams/Trigrams/Fourgrams**: 2/3/4-letter sequence frequencies
- **Positional patterns**: Where specific sequences appear (start/middle/end)
- **Markov chains**: Context → next letter probabilities
  - Example: `"st": {"r": 3, "a": 1}` means after "st", "r" appears 3 times, "a" once

### Syllable Analysis (if enabled)
- **all_syllables**: Complete list of unique syllables found
- **syllable_frequencies**: How often each syllable appears
- **positional_syllables**: Syllables by position in word
- **syllable_markov**: Syllable-to-syllable transitions

### Component Analysis (if enabled)
- **Onset**: Initial consonant cluster (can be empty)
- **Nucleus**: Vowel or vowel group (required)
- **Coda**: Final consonant cluster (can be empty)
- Example: "strength" → onset="str", nucleus="e", coda="ngth"

## Workflow: Creating Name Profiles

### Step 1: Collect Word Lists
Create themed word lists for different name styles:

```bash
# Greek mythology names
echo -e "zeus\nathena\napollo\nartemis\nposeidon" > greek_gods.txt

# Norse mythology names
echo -e "thor\nodin\nloki\nfreya\ntyr" > norse_gods.txt

# Fantasy-sounding English words
echo -e "dragon\nthunder\nshadow\nstorm\nflame" > fantasy_words.txt
```

### Step 2: Analyze Each Corpus
Generate statistical profiles:

```bash
# Greek profile (full analysis)
./build/nameanalyzer greek_gods.txt -o profiles/greek.json \
  --markov-order 2 --enable-syllables --enable-components

# Norse profile
./build/nameanalyzer norse_gods.txt -o profiles/norse.json \
  --markov-order 2 --enable-syllables --enable-components

# Fantasy profile
./build/nameanalyzer fantasy_words.txt -o profiles/fantasy.json \
  --markov-order 3 --enable-syllables --enable-components
```

### Step 3: Use with NameGenerator
Feed the JSON profiles to NameGenerator to create new names with the captured style:

```bash
# Example (when NameGenerator is ready)
./namegenerator --profile profiles/greek.json --count 10
# Output: Zeusara, Athemon, Poseidera, ...

./namegenerator --profile profiles/norse.json --count 10
# Output: Thorvald, Odinson, Freygar, ...
```

## Advanced Tips

### Larger Corpora = Better Results
- Minimum recommended: 50-100 words
- Ideal: 500+ words for rich patterns
- More data captures rarer but valid combinations

### Combining Multiple Sources
Analyze related word lists together:
```bash
cat greek_gods.txt greek_heroes.txt greek_places.txt > all_greek.txt
./build/nameanalyzer all_greek.txt -o greek_complete.json \
  --markov-order 2 --enable-syllables --enable-components
```

### Tuning Markov Order
- **Order 1**: More random, creative combinations
- **Order 2**: Balanced (recommended for most uses)
- **Order 3**: More faithful to original corpus, less variation

### Finding Word Lists
Good sources for themed word lists:
- Mythology databases (Greek, Norse, Egyptian, Celtic)
- Fantasy name lists from literature
- Place names (cities, mountains, rivers)
- Dictionary words filtered by theme
- Historical name databases

## Testing

A test word list is included:

```bash
./build/nameanalyzer test_words.txt -o test_output.json \
  --markov-order 2 --enable-syllables --enable-components -v
```

Expected output:
- 15 words analyzed
- ~30 unique syllables
- ~17 onsets, ~8 nuclei, ~9 codas
- Valid JSON output

Verify JSON validity:
```bash
python3 -m json.tool test_output.json > /dev/null && echo "Valid JSON!"
```

## Troubleshooting

**Error: "No valid words found in file"**
- Check file encoding (must be UTF-8)
- Ensure words meet minimum length requirement
- Verify file has content and proper line endings

**Error: "Failed to open file"**
- Check file path is correct
- Verify file permissions
- Use absolute path if relative path fails

**Syllable detection seems off**
- Heuristic algorithm ~85-90% accurate for English
- Works best with real words, not abbreviations
- Some complex words may be split unexpectedly
- Consider this a feature: creative syllable discovery!

## Technical Details

### Syllable Detection Algorithm
Uses heuristic rules to split words:
1. Identify vowel groups (nuclei)
2. Distribute consonants between vowels
3. Apply common patterns (V-CV, VC-CV, VC-CCV)
4. Extract onset/nucleus/coda for each syllable

### Markov Chain Format
- Context markers: `^` = start of word, `$` = end of word
- Example: 2nd-order chain for "strong"
  - `^^` → `s` (word starts with 's')
  - `^s` → `t`
  - `st` → `r`
  - `tr` → `o`
  - `ro` → `n`
  - `on` → `g`
  - `ng` → `$` (word ends after 'ng')

## License

See LICENSE file for details.

## Contributing

This tool is designed to work with NameGenerator. When creating profiles for NameGenerator, consider:
- Curating themed word lists carefully
- Analyzing multiple related corpora separately
- Experimenting with different Markov orders
- Sharing interesting profiles you create!

## See Also

- **NameGenerator** - Companion tool that uses these profiles to generate names
- Create profiles for different cultures, time periods, or fantasy settings
- Combine profiles for hybrid name styles
