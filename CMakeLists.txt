cmake_minimum_required(VERSION 3.20)
project(NameAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# No external dependencies needed - JSON output is written manually

# Source files
set(SOURCES
    src/main.cpp
    src/word_reader.cpp
    src/ngram_extractor.cpp
    src/markov_builder.cpp
    src/syllable_detector.cpp
    src/component_extractor.cpp
    src/json_writer.cpp
    src/cli_parser.cpp
)

# Header files
set(HEADERS
    include/word_reader.hpp
    include/ngram_extractor.hpp
    include/markov_builder.hpp
    include/syllable_detector.hpp
    include/component_extractor.hpp
    include/json_writer.hpp
    include/cli_parser.hpp
    include/types.hpp
)

# Executable
add_executable(nameanalyzer ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(nameanalyzer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# No external libraries to link

# Compiler warnings
if(MSVC)
    target_compile_options(nameanalyzer PRIVATE /W4)
else()
    target_compile_options(nameanalyzer PRIVATE -Wall -Wextra -Wpedantic)
endif()
