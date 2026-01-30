SHELL := /bin/bash
.DEFAULT_GOAL := help

# Directories
BUILD_DIR := build
BUILD_WASM_DIR := build-wasm
INCLUDE_DIR := include
SRC_DIR := src
THIRD_PARTY := third_party
TESTS_DIR := tests
WEB_DIR := web
WASM_OUTPUT_DIR := $(WEB_DIR)/public/wasm

# Emscripten
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

.PHONY: help check-deps deps init configure build run clean rebuild format lint test \
        wasm-init wasm-build wasm-clean web-init web-dev web-build web-clean all-clean

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
	@printf "  check-deps    Verify all required tools are installed\n"
	@printf "  deps          Download all third-party dependencies\n"
	@printf "  init          Full project initialization (deps + configure)\n"
	@printf "\n"
	@printf "$(YELLOW)Native Build:$(NC)\n"
	@printf "  build         Compile the native executable\n"
	@printf "  run           Build and run the executable\n"
	@printf "  clean         Remove native build artifacts\n"
	@printf "  rebuild       Clean and rebuild everything\n"
	@printf "\n"
	@printf "$(YELLOW)Code Quality:$(NC)\n"
	@printf "  format        Format all source files with clang-format\n"
	@printf "  lint          Run clang-tidy on all source files\n"
	@printf "  test          Run all tests\n"
	@printf "\n"
	@printf "$(YELLOW)WebAssembly:$(NC)\n"
	@printf "  wasm-init     Configure Emscripten build\n"
	@printf "  wasm-build    Compile to WebAssembly\n"
	@printf "  wasm-clean    Remove WASM build artifacts\n"
	@printf "\n"
	@printf "$(YELLOW)Frontend:$(NC)\n"
	@printf "  web-init      Initialize frontend (bun install)\n"
	@printf "  web-dev       Start frontend dev server\n"
	@printf "  web-build     Build frontend for production\n"
	@printf "  web-clean     Remove frontend build artifacts\n"
	@printf "\n"
	@printf "$(YELLOW)Combined:$(NC)\n"
	@printf "  all-clean     Remove all build artifacts\n"
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
	@# Emscripten
	@if [ -f "$(EMSDK_ENV)" ]; then \
		printf "  $(CHECK) emscripten:   $(EMSDK_ENV)\n"; \
	else \
		printf "  $(CROSS) emscripten:   not found at $(EMSDK_ENV)\n"; \
	fi
	@# Bun
	@command -v bun >/dev/null 2>&1 && \
		printf "  $(CHECK) bun:          $$(bun --version)\n" || \
		printf "  $(CROSS) bun:          not found (curl -fsSL https://bun.sh/install | bash)\n"
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
	@# spdlog
	@if [ -f "$(THIRD_PARTY)/spdlog/include/spdlog/spdlog.h" ]; then \
		printf "  $(CHECK) spdlog:       $(THIRD_PARTY)/spdlog/\n"; \
	else \
		printf "  $(CROSS) spdlog:       not found (run: make deps)\n"; \
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
	@# spdlog (header-only)
	@if [ ! -f "$(THIRD_PARTY)/spdlog/include/spdlog/spdlog.h" ]; then \
		printf "  $(ARROW) Downloading spdlog 1.14.1...\n"; \
		mkdir -p $(THIRD_PARTY)/spdlog; \
		curl -sL https://github.com/gabime/spdlog/archive/refs/tags/v1.14.1.tar.gz | \
			tar xz -C $(THIRD_PARTY)/spdlog --strip-components=1; \
		printf "  $(CHECK) spdlog installed\n"; \
	else \
		printf "  $(CHECK) spdlog already installed\n"; \
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
# Native Build
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

## run: Build and run the executable
run: build
	@./$(BUILD_DIR)/defect_gnn

## clean: Remove build artifacts
clean:
	@printf "\n"
	@printf "$(BLUE)Cleaning native build artifacts...$(NC)\n"
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
# WebAssembly
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
	@mkdir -p $(BUILD_WASM_DIR)
	@mkdir -p $(WASM_OUTPUT_DIR)
	@source $(EMSDK_ENV) && \
		emcmake cmake -B $(BUILD_WASM_DIR) -G Ninja \
			-DCMAKE_BUILD_TYPE=Release
	@printf "\n"
	@printf "$(GREEN)WASM build configured!$(NC)\n"
	@printf "\n"

## wasm-build: Compile to WebAssembly
wasm-build:
	@printf "\n"
	@printf "$(BLUE)Building WebAssembly...$(NC)\n"
	@printf "\n"
	@if [ ! -d "$(BUILD_WASM_DIR)" ]; then \
		printf "$(RED)WASM build not configured. Run 'make wasm-init' first.$(NC)\n"; \
		exit 1; \
	fi
	@source $(EMSDK_ENV) && cmake --build $(BUILD_WASM_DIR)
	@printf "\n"
	@printf "$(BLUE)Copying WASM files to web/public/wasm/...$(NC)\n"
	@mkdir -p $(WASM_OUTPUT_DIR)
	@cp $(BUILD_WASM_DIR)/defect_gnn_viz.js $(WASM_OUTPUT_DIR)/
	@cp $(BUILD_WASM_DIR)/defect_gnn_viz.wasm $(WASM_OUTPUT_DIR)/
	@printf "  $(CHECK) defect_gnn_viz.js\n"
	@printf "  $(CHECK) defect_gnn_viz.wasm\n"
	@printf "\n"
	@printf "$(GREEN)WASM build complete!$(NC)\n"
	@printf "\n"

## wasm-clean: Remove WASM build artifacts
wasm-clean:
	@printf "\n"
	@printf "$(BLUE)Cleaning WASM build artifacts...$(NC)\n"
	@rm -rf $(BUILD_WASM_DIR)
	@rm -f $(WASM_OUTPUT_DIR)/defect_gnn_viz.*
	@printf "  $(CHECK) WASM build directory removed\n"
	@printf "\n"

#==============================================================================
# Frontend
#==============================================================================

## web-init: Initialize frontend (bun install)
web-init:
	@printf "\n"
	@printf "$(BLUE)Initializing frontend...$(NC)\n"
	@printf "\n"
	@if [ ! -d "$(WEB_DIR)" ]; then \
		printf "$(RED)Web directory not found. Create web/ first.$(NC)\n"; \
		exit 1; \
	fi
	@cd $(WEB_DIR) && bun install
	@printf "\n"
	@printf "$(GREEN)Frontend initialized!$(NC)\n"
	@printf "\n"

## web-dev: Start frontend dev server
web-dev:
	@printf "\n"
	@printf "$(BLUE)Starting frontend dev server...$(NC)\n"
	@printf "\n"
	@cd $(WEB_DIR) && bun run dev

## web-build: Build frontend for production
web-build: wasm-build
	@printf "\n"
	@printf "$(BLUE)Building frontend for production...$(NC)\n"
	@printf "\n"
	@cd $(WEB_DIR) && bun run build
	@printf "\n"
	@printf "$(GREEN)Frontend build complete!$(NC)\n"
	@printf "\n"

## web-clean: Remove frontend build artifacts
web-clean:
	@printf "\n"
	@printf "$(BLUE)Cleaning frontend build artifacts...$(NC)\n"
	@rm -rf $(WEB_DIR)/.next
	@rm -rf $(WEB_DIR)/out
	@rm -rf $(WEB_DIR)/node_modules
	@printf "  $(CHECK) Frontend build artifacts removed\n"
	@printf "\n"

#==============================================================================
# Combined
#==============================================================================

## all-clean: Remove all build artifacts
all-clean: clean wasm-clean web-clean
	@printf "$(GREEN)All build artifacts removed!$(NC)\n"
	@printf "\n"