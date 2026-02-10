cmake_minimum_required(VERSION 3.16)
project(defect_gnn
    VERSION 0.1.0
    DESCRIPTION "Defect Formation Energy Prediction using GNN + Persistent Homology"
    LANGUAGES CXX
)

# ============================================================================
# Build Options
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
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

# spdlog (header-only logging)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/spdlog/include/spdlog/spdlog.h")
    set(SPDLOG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/spdlog/include")
    message(STATUS "Using bundled spdlog")
else()
    message(STATUS "spdlog not found. Download to third_party/spdlog/")
endif()

# Ripser (persistent homology - we'll use it as a library)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/ripser/ripser.cpp")
    set(RIPSER_DIR "${CMAKE_SOURCE_DIR}/third_party/ripser")
    message(STATUS "Using bundled Ripser")
else()
    message(STATUS "Ripser not found. Clone to third_party/ripser/")
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

if(SPDLOG_INCLUDE_DIR)
    include_directories(${SPDLOG_INCLUDE_DIR})
endif()

if(RIPSER_DIR)
    include_directories(${RIPSER_DIR})
endif()

# ============================================================================
# Source Files
# ============================================================================

# Header files (for IDE integration)
file(GLOB_RECURSE HEADERS
    "include/*.hpp"
)

# ============================================================================
# Emscripten (WASM) Build
# ============================================================================
if(EMSCRIPTEN)
    message(STATUS "")
    message(STATUS "=== Building for WebAssembly ===")

    # Visualization-only sources (no training, no Ripser)
    set(WASM_SOURCES
        src/io/vasp_parser.cpp
        src/crystal/structure.cpp
        src/graph/neighbor_list.cpp
        src/graph/edge_features.cpp
        src/viz/wasm_bindings.cpp
    )

    add_executable(defect_gnn_viz ${WASM_SOURCES} ${HEADERS})

    target_include_directories(defect_gnn_viz PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    # Emscripten-specific linker flags
    set_target_properties(defect_gnn_viz PROPERTIES
        SUFFIX ".js"
        LINK_FLAGS "\
            -sMODULARIZE=1 \
            -sEXPORT_NAME='createDefectGNNModule' \
            -sALLOW_MEMORY_GROWTH=1 \
            -sENVIRONMENT='web' \
            -sNO_EXIT_RUNTIME=1 \
            -sINVOKE_RUN=0 \
            --bind \
            -O3 \
            -flto \
        "
    )

    message(STATUS "WASM target: defect_gnn_viz")
    message(STATUS "Output: defect_gnn_viz.js + defect_gnn_viz.wasm")

# ============================================================================
# Native Build
# ============================================================================
else()
    # Collect all source files excluding WASM bindings
    file(GLOB_RECURSE SOURCES
        "src/*.cpp"
    )
    # Remove wasm_bindings.cpp and preprocess_betti.cpp from native build
    list(FILTER SOURCES EXCLUDE REGEX ".*wasm_bindings\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*preprocess_betti\\.cpp$")

    if(SOURCES)
        add_executable(defect_gnn ${SOURCES} ${HEADERS})
        target_include_directories(defect_gnn PRIVATE
            ${CMAKE_SOURCE_DIR}/include
        )
    else()
        message(STATUS "No source files found in src/ yet. Skipping executable target.")
    endif()

    # ========================================================================
    # Preprocessing Executable
    # ========================================================================
    add_executable(preprocess_betti
        src/preprocess/preprocess_betti.cpp
        src/io/vasp_parser.cpp
        src/crystal/structure.cpp
        src/graph/neighbor_list.cpp
        src/topology/ripser_wrapper.cpp
        src/topology/betti_features.cpp
        src/topology/pca.cpp
    )
    target_include_directories(preprocess_betti PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    # ========================================================================
    # Tests
    # ========================================================================
    option(BUILD_TESTS "Build unit tests" ON)

    if(BUILD_TESTS)
        enable_testing()

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

    # ========================================================================
    # Symlink compile_commands.json to project root for clangd
    # ========================================================================
    if(CMAKE_EXPORT_COMPILE_COMMANDS)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_BINARY_DIR}/compile_commands.json
                ${CMAKE_SOURCE_DIR}/compile_commands.json
        )
    endif()
endif()

# ============================================================================
# Print Configuration Summary
# ============================================================================
message(STATUS "")
message(STATUS "=== Defect-GNN Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(EMSCRIPTEN)
    message(STATUS "Target: WebAssembly")
else()
    message(STATUS "Target: Native")
endif()
message(STATUS "")