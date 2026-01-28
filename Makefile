SHELL := /bin/bash
.DEFAULT_GOAL := help

# Directories
BUILD_DIR := build
INCLUDE_DIR := include
SRC_DIR := src
THIRD_PARTY := third_party
TESTS_DIR := tests

# Emscripten (for WebAssembly phase)
EMSDK_ENV := $(HOME)/emsdk/emsdk_env.sh

# Colors for output
GREEN := \033[0;32m
RED := \033[0;31m
BLUE := \033[0;34m
YELLOW := \033[0;33m
CYAN := \033[0;36m
NC := \033[0m

# Symbols
CHECK := $(GREEN)✓$(NC)
CROSS := $(RED)✗$(NC)
ARROW := $(CYAN)→$(NC)

.PHONY: help check-deps deps init configure build clean rebuild format lint test wasm-init wasm-build

#==============================================================================
# Help
#==============================================================================

## help: Show this help message
help:
	@printf "\n"
	@printf "$(CYAN)════════════════════════════════════════════════════════════════$(NC)\n"
	@printf "$(CYAN)  Defect-GNN C++ - Development Commands$(NC)\n"
	@printf "$(CYAN)════════════════════════════════════════════════════════════════$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)Setup:$(NC)\n"
	@grep -E '^## (check-deps|deps|init|configure):' $(MAKEFILE_LIST) | sed 's/## /  /' | column -t -s ':'
	@printf "\n"
	@printf "$(YELLOW)Build:$(NC)\n"
	@grep -E '^## (build|clean|rebuild):' $(MAKEFILE_LIST) | sed 's/## /  /' | column -t -s ':'
	@printf "\n"
	@printf "$(YELLOW)Code Quality:$(NC)\n"
	@grep -E '^## (format|lint|test):' $(MAKEFILE_LIST) | sed 's/## /  /' | column -t -s ':'
	@printf "\n"
	@printf "$(YELLOW)WebAssembly:$(NC)\n"
	@grep -E '^## (wasm-):' $(MAKEFILE_LIST) | sed 's/## /  /' | column -t -s ':'
	@printf "\n"

#==============================================================================
# Dependency Checking
#==============================================================================

## check-deps: Verify all required tools are installed
check-deps:
	@printf "\n"
	@printf "$(BLUE)Checking dependencies...$(NC)\n"
	@printf "\n"
	@# CMake
	@command -v cmake >/dev/null 2>&1 && \
		printf "  $(CHECK) cmake:        $$(cmake --version | head -1)\n" || \
		printf "  $(CROSS) cmake:        not found (apt install cmake)\n"
	@# Ninja
	@command -v ninja >/dev/null 2>&1 && \
		printf "  $(CHECK) ninja:        $$(ninja --version)\n" || \
		printf "  $(CROSS) ninja:        not found (apt install ninja-build)\n"
	@# Clang
	@command -v clang++ >/dev/null 2>&1 && \
		printf "  $(CHECK) clang++:      $$(clang++ --version | head -1)\n" || \
		printf "  $(CROSS) clang++:      not found (apt install clang)\n"
	@# clang-format
	@command -v clang-format >/dev/null 2>&1 && \
		printf "  $(CHECK) clang-format: $$(clang-format --version)\n" || \
		printf "  $(CROSS) clang-format: not found (apt install clang-format)\n"
	@# clang-tidy
	@command -v clang-tidy >/dev/null 2>&1 && \
		printf "  $(CHECK) clang-tidy:   $$(clang-tidy --version | head -1)\n" || \
		printf "  $(CROSS) clang-tidy:   not found (apt install clang-tidy)\n"
	@# clangd
	@command -v clangd >/dev/null 2>&1 && \
		printf "  $(CHECK) clangd:       $$(clangd --version | head -1)\n" || \
		printf "  $(CROSS) clangd:       not found (apt install clangd)\n"
	@printf "\n"
	@printf "$(BLUE)Checking third-party libraries...$(NC)\n"
	@printf "\n"
	@# Eigen
	@if [ -f "$(THIRD_PARTY)/eigen/Eigen/Core" ]; then \
		printf "  $(CHECK) Eigen:        $(THIRD_PARTY)/eigen/\n"; \
	elif [ -f "/usr/include/eigen3/Eigen/Core" ]; then \
		printf "  $(CHECK) Eigen:        /usr/include/eigen3/ (system)\n"; \
	else \
		printf "  $(CROSS) Eigen:        not found (run: make deps)\n"; \
	fi
	@# nlohmann/json
	@if [ -f "$(THIRD_PARTY)/nlohmann_json/nlohmann/json.hpp" ]; then \
		printf "  $(CHECK) nlohmann/json: $(THIRD_PARTY)/nlohmann_json/\n"; \
	else \
		printf "  $(CROSS) nlohmann/json: not found (run: make deps)\n"; \
	fi
	@# nanoflann
	@if [ -f "$(THIRD_PARTY)/nanoflann/nanoflann.hpp" ]; then \
		printf "  $(CHECK) nanoflann:    $(THIRD_PARTY)/nanoflann/\n"; \
	else \
		printf "  $(CROSS) nanoflann:    not found (run: make deps)\n"; \
	fi
	@# Ripser
	@if [ -f "$(THIRD_PARTY)/ripser/ripser.cpp" ]; then \
		printf "  $(CHECK) Ripser:       $(THIRD_PARTY)/ripser/\n"; \
	else \
		printf "  $(CROSS) Ripser:       not found (run: make deps)\n"; \
	fi
	@printf "\n"

#==============================================================================
# Setup
#==============================================================================

## deps: Download all third-party dependencies
deps:
	@printf "\n"
	@printf "$(BLUE)Downloading third-party dependencies...$(NC)\n"
	@printf "\n"
	@mkdir -p $(THIRD_PARTY)
	@# Eigen
	@if [ ! -f "$(THIRD_PARTY)/eigen/Eigen/Core" ]; then \
		printf "  $(ARROW) Downloading Eigen 3.4.0...\n"; \
		mkdir -p $(THIRD_PARTY)/eigen; \
		curl -sL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz | \
			tar xz -C $(THIRD_PARTY)/eigen --strip-components=1; \
		printf "  $(CHECK) Eigen installed\n"; \
	else \
		printf "  $(CHECK) Eigen already installed\n"; \
	fi
	@# nlohmann/json (single header)
	@if [ ! -f "$(THIRD_PARTY)/nlohmann_json/nlohmann/json.hpp" ]; then \
		printf "  $(ARROW) Downloading nlohmann/json...\n"; \
		mkdir -p $(THIRD_PARTY)/nlohmann_json/nlohmann; \
		curl -sL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp \
			-o $(THIRD_PARTY)/nlohmann_json/nlohmann/json.hpp; \
		printf "  $(CHECK) nlohmann/json installed\n"; \
	else \
		printf "  $(CHECK) nlohmann/json already installed\n"; \
	fi
	@# nanoflann (single header)
	@if [ ! -f "$(THIRD_PARTY)/nanoflann/nanoflann.hpp" ]; then \
		printf "  $(ARROW) Downloading nanoflann...\n"; \
		mkdir -p $(THIRD_PARTY)/nanoflann; \
		curl -sL https://raw.githubusercontent.com/jlblancoc/nanoflann/v1.5.5/include/nanoflann.hpp \
			-o $(THIRD_PARTY)/nanoflann/nanoflann.hpp; \
		printf "  $(CHECK) nanoflann installed\n"; \
	else \
		printf "  $(CHECK) nanoflann already installed\n"; \
	fi
	@# Ripser (persistent homology)
	@if [ ! -f "$(THIRD_PARTY)/ripser/ripser.cpp" ]; then \
		printf "  $(ARROW) Downloading Ripser...\n"; \
		git clone --depth 1 https://github.com/Ripser/ripser.git $(THIRD_PARTY)/ripser; \
		rm -rf $(THIRD_PARTY)/ripser/.git; \
		printf "  $(CHECK) Ripser installed\n"; \
	else \
		printf "  $(CHECK) Ripser already installed\n"; \
	fi
	@printf "\n"
	@printf "$(GREEN)All dependencies installed!$(NC)\n"
	@printf "\n"

## configure: Configure CMake build system
configure:
	@printf "\n"
	@printf "$(BLUE)Configuring CMake...$(NC)\n"
	@printf "\n"
	@mkdir -p $(BUILD_DIR)
	@cmake -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_CXX_COMPILER=clang++
	@printf "\n"
	@printf "$(GREEN)Configuration complete!$(NC)\n"
	@printf "  $(ARROW) compile_commands.json symlinked to project root\n"
	@printf "\n"

## init: Full project initialization (deps + configure)
init: deps configure
	@printf "$(GREEN)Project initialized!$(NC)\n"
	@printf "  $(ARROW) Run 'make build' to compile\n"
	@printf "  $(ARROW) Restart VSCode for clangd to pick up changes\n"
	@printf "\n"

#==============================================================================
# Build
#==============================================================================

## build: Compile the project
build:
	@printf "\n"
	@printf "$(BLUE)Building...$(NC)\n"
	@printf "\n"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		printf "$(RED)Build directory not found. Run 'make init' first.$(NC)\n"; \
		exit 1; \
	fi
	@cmake --build $(BUILD_DIR)
	@printf "\n"
	@printf "$(GREEN)Build complete!$(NC)\n"
	@printf "\n"

## clean: Remove build artifacts
clean:
	@printf "\n"
	@printf "$(BLUE)Cleaning build artifacts...$(NC)\n"
	@rm -rf $(BUILD_DIR)
	@rm -f compile_commands.json
	@printf "  $(CHECK) Build directory removed\n"
	@printf "\n"

## rebuild: Clean and rebuild everything
rebuild: clean init build
	@printf "$(GREEN)Full rebuild complete!$(NC)\n"
	@printf "\n"

#==============================================================================
# Code Quality
#==============================================================================

## format: Format all source files with clang-format
format:
	@printf "\n"
	@printf "$(BLUE)Formatting source files...$(NC)\n"
	@printf "\n"
	@find $(INCLUDE_DIR) $(SRC_DIR) $(TESTS_DIR) -name '*.hpp' -o -name '*.cpp' 2>/dev/null | \
		xargs -r clang-format -i --style=file
	@printf "  $(CHECK) All files formatted\n"
	@printf "\n"

## lint: Run clang-tidy on all source files
lint:
	@printf "\n"
	@printf "$(BLUE)Running clang-tidy...$(NC)\n"
	@printf "\n"
	@if [ ! -f "compile_commands.json" ]; then \
		printf "$(RED)compile_commands.json not found. Run 'make configure' first.$(NC)\n"; \
		exit 1; \
	fi
	@find $(SRC_DIR) -name '*.cpp' 2>/dev/null | \
		xargs -r clang-tidy -p $(BUILD_DIR) 2>/dev/null || true
	@printf "\n"
	@printf "$(GREEN)Linting complete!$(NC)\n"
	@printf "\n"

## test: Run all tests
test:
	@printf "\n"
	@printf "$(BLUE)Running tests...$(NC)\n"
	@printf "\n"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		printf "$(RED)Build directory not found. Run 'make build' first.$(NC)\n"; \
		exit 1; \
	fi
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@printf "\n"

#==============================================================================
# WebAssembly (Phase 6)
#==============================================================================

## wasm-init: Initialize Emscripten build
wasm-init:
	@printf "\n"
	@printf "$(BLUE)Initializing WebAssembly build...$(NC)\n"
	@printf "\n"
	@if [ ! -f "$(EMSDK_ENV)" ]; then \
		printf "$(RED)Emscripten not found at $(EMSDK_ENV)$(NC)\n"; \
		printf "Install from: https://emscripten.org/docs/getting_started/downloads.html\n"; \
		exit 1; \
	fi
	@mkdir -p $(BUILD_DIR)-wasm
	@source $(EMSDK_ENV) && \
		emcmake cmake -B $(BUILD_DIR)-wasm -G Ninja \
			-DCMAKE_BUILD_TYPE=Release
	@printf "\n"
	@printf "$(GREEN)WASM build configured!$(NC)\n"
	@printf "\n"

## wasm-build: Compile to WebAssembly
wasm-build:
	@printf "\n"
	@printf "$(BLUE)Building WebAssembly...$(NC)\n"
	@printf "\n"
	@if [ ! -d "$(BUILD_DIR)-wasm" ]; then \
		printf "$(RED)WASM build not configured. Run 'make wasm-init' first.$(NC)\n"; \
		exit 1; \
	fi
	@source $(EMSDK_ENV) && cmake --build $(BUILD_DIR)-wasm
	@printf "\n"
	@printf "$(GREEN)WASM build complete!$(NC)\n"
	@printf "\n"
