#pragma once
#include <filesystem>
#include <string>

namespace DoEngine {

    class PathUtils {
    public:
        // Returns the absolute path of the directory containing the currently running executable
        static std::filesystem::path GetExecutableDirectory();

        // Returns the full path to the running executable
        static std::filesystem::path GetExecutablePath();

        // Walks up from the executable directory until it finds the engine's root (where shaders/ exists)
        static std::filesystem::path GetEngineRoot();

        // Converts a relative project path to an absolute path based on the engine root
        static std::filesystem::path GetEngineAssetPath(const std::string& relativePath);
    };

}
