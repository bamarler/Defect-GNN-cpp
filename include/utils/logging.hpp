#pragma once

#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

namespace defect_gnn::utils {

inline void init_logger(const std::string& name) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(name + ".log");

    auto logger =
        std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{console_sink, file_sink});

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
}

}  // namespace defect_gnn::utils