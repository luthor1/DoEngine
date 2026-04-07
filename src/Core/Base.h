#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>

// Common macros and aliases
namespace DoEngine {
    using uint = unsigned int;
    using int32 = int;
    using uint32 = unsigned int;
    using uint64 = unsigned long long;
    
    // Debug logging macros
    #ifdef DO_DEBUG
        #define DO_LOG(x) std::cout << "[DO_ENGINE] " << x << std::endl
    #else
        #define DO_LOG(x)
    #endif
}
