#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

// Common macros and aliases
namespace DoEngine {
    using uint = unsigned int;
    using int32 = int;
    using uint32 = unsigned int;
    using uint64 = unsigned long long;
    
    // Core Logging System
    #define DO_CORE_TRACE(...)    spdlog::trace(__VA_ARGS__)
    #define DO_CORE_INFO(...)     spdlog::info(__VA_ARGS__)
    #define DO_CORE_WARN(...)     spdlog::warn(__VA_ARGS__)
    #define DO_CORE_ERROR(...)    spdlog::error(__VA_ARGS__)
    #define DO_CORE_CRITICAL(...) spdlog::critical(__VA_ARGS__)

    // Legacy support (to be phased out)
    #define DO_LOG(x)             spdlog::info(x)
    #define DO_LOG_ERROR(x)       spdlog::error(x)
}
