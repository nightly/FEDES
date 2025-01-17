﻿cmake_minimum_required (VERSION 3.22)
project ("fedes" VERSION 0 LANGUAGES CXX)

option(FEDES_RUN_TESTS "Run tests" ON)
option(FEDES_COMPILE_BENCHMARKS "Compile benchmarks" ON)

option(FEDES_VERBOSE_OUTPUT "Verbose output" OFF)
option(FEDES_STATS "Collect and run stats during exection" OFF)

# Library
add_subdirectory(src/fedes)

# Tests
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND FEDES_RUN_TESTS)
    include(CTest)
    enable_testing()
    include(GoogleTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(tests)
endif()

# Applications
add_subdirectory(apps/cli)

# Benchmarks
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND FEDES_COMPILE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Preprocessor definitions
if (FEDES_VERBOSE_OUTPUT)
    target_compile_definitions(fedes PRIVATE FEDES_VERBOSE=1)
    target_compile_definitions(app_fedes_cli PRIVATE FEDES_VERBOSE=1)
endif()

if (FEDES_STATS)
    target_compile_definitions(fedes PRIVATE FEDES_STATS=1)
    target_compile_definitions(app_fedes_cli PRIVATE FEDES_STATS=1)
endif()