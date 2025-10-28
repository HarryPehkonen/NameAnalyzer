cmake_minimum_required(VERSION 3.20)
project(NameAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# JSOM dependency - try to find locally first, then fetch if needed
find_package(jsom QUIET)

if(NOT jsom_FOUND)
    message(STATUS "JSOM not found locally, fetching from GitHub...")
    include(FetchContent)

    FetchContent_Declare(
        JSOM
        GIT_REPOSITORY https://github.com/HarryPehkonen/JSOM.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )

    FetchContent_GetProperties(JSOM)
    if(NOT jsom_POPULATED)
        FetchContent_Populate(JSOM)

        # Build minimal JSOM library (avoids benchmarks/tests)
        add_library(jsom_lib
            ${jsom_SOURCE_DIR}/src/json_document_pointer.cpp
            ${jsom_SOURCE_DIR}/src/json_document_formatting.cpp
        )

        target_include_directories(jsom_lib PUBLIC
            ${jsom_SOURCE_DIR}/include
        )

        set_target_properties(jsom_lib PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
        )
    endif()
else()
    message(STATUS "Using locally installed JSOM")
endif()

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

# Link JSOM library
target_link_libraries(nameanalyzer PRIVATE jsom_lib)

# Compiler warnings
if(MSVC)
    target_compile_options(nameanalyzer PRIVATE /W4)
else()
    target_compile_options(nameanalyzer PRIVATE -Wall -Wextra -Wpedantic)
endif()
