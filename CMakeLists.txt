cmake_minimum_required(VERSION 3.16)
project(defect_gnn
    VERSION 0.1.0
    DESCRIPTION "Defect Formation Energy Prediction using GNN + Persistent Homology"
    LANGUAGES CXX
)

# ============================================================================
# Build Options
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type (Debug, Release, RelWithDebInfo)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# ============================================================================
# Compiler Warnings
# ============================================================================
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter  # Eigen triggers this a lot
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O3 -DNDEBUG)
endif()

# ============================================================================
# Dependencies
# ============================================================================

# Eigen (header-only)
# Option 1: System install
find_package(Eigen3 3.3 QUIET)
if(NOT Eigen3_FOUND)
    # Option 2: Bundled in third_party/
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/eigen/Eigen/Core")
        message(STATUS "Using bundled Eigen from third_party/eigen")
        set(EIGEN3_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/eigen")
    else()
        message(STATUS "Eigen not found. Install with: sudo apt install libeigen3-dev")
        message(STATUS "Or download to third_party/eigen from: https://eigen.tuxfamily.org")
    endif()
else()
    message(STATUS "Found system Eigen: ${EIGEN3_INCLUDE_DIR}")
endif()

# nlohmann/json (header-only)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/nlohmann/json.hpp")
    set(NLOHMANN_JSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/nlohmann_json")
    message(STATUS "Using bundled nlohmann/json")
else()
    find_package(nlohmann_json QUIET)
    if(NOT nlohmann_json_FOUND)
        message(STATUS "nlohmann/json not found. Download single header to third_party/nlohmann_json/nlohmann/json.hpp")
    endif()
endif()

# nanoflann (header-only, for KD-tree neighbor search)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanoflann/nanoflann.hpp")
    set(NANOFLANN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanoflann")
    message(STATUS "Using bundled nanoflann")
else()
    message(STATUS "nanoflann not found. Download to third_party/nanoflann/")
endif()

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

if(EIGEN3_INCLUDE_DIR)
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

if(NLOHMANN_JSON_INCLUDE_DIR)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

if(NANOFLANN_INCLUDE_DIR)
    include_directories(${NANOFLANN_INCLUDE_DIR})
endif()

# ============================================================================
# Source Files
# ============================================================================

# Collect all source files (will be populated as you add them)
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

# Header files (for IDE integration)
file(GLOB_RECURSE HEADERS
    "include/*.hpp"
)

# ============================================================================
# Main Executable
# ============================================================================
if(SOURCES)
    add_executable(defect_gnn ${SOURCES} ${HEADERS})
    target_include_directories(defect_gnn PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
else()
    message(STATUS "No source files found in src/ yet. Skipping executable target.")
endif()

# ============================================================================
# Tests
# ============================================================================
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()

    # Test files
    file(GLOB TEST_SOURCES "tests/*.cpp")

    if(TEST_SOURCES)
        foreach(TEST_FILE ${TEST_SOURCES})
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            add_executable(${TEST_NAME} ${TEST_FILE})
            target_include_directories(${TEST_NAME} PRIVATE
                ${CMAKE_SOURCE_DIR}/include
            )
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endforeach()
    endif()
endif()

# ============================================================================
# Symlink compile_commands.json to project root for clangd
# ============================================================================
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()

# ============================================================================
# Print Configuration Summary
# ============================================================================
message(STATUS "")
message(STATUS "=== Defect-GNN Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
