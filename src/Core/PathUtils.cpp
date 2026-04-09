#include "PathUtils.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <algorithm>
#include <iostream>
#include "Base.h"

namespace DoEngine {

    std::filesystem::path PathUtils::GetExecutablePath() {
#ifdef _WIN32
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        return std::filesystem::path(path);
#else
        // Fallback for Linux (readlink /proc/self/exe) could be added here
        return std::filesystem::current_path();
#endif
    }

    std::filesystem::path PathUtils::GetExecutableDirectory() {
        return GetExecutablePath().parent_path();
    }

    std::filesystem::path PathUtils::GetEngineRoot() {
        static std::filesystem::path s_EngineRoot = "";
        if (!s_EngineRoot.empty()) return s_EngineRoot;

        std::filesystem::path current = GetExecutableDirectory();
        
        // Unreal/Godot strategy: Walk up until we find a signature folder (like "shaders" or "src")
        // We limit search to 5 levels to avoid scanning the entire drive
        for (int i = 0; i < 5; ++i) {
            if (std::filesystem::exists(current / "shaders") && std::filesystem::exists(current / "src")) {
                s_EngineRoot = current;
                return s_EngineRoot;
            }
            if (current.has_parent_path()) {
                current = current.parent_path();
            } else {
                break;
            }
        }

        // Fallback to current directory if discovery fails
        return std::filesystem::current_path();
    }

    std::filesystem::path PathUtils::GetEngineAssetPath(const std::string& relativePath) {
        return GetEngineRoot() / relativePath;
    }

}
