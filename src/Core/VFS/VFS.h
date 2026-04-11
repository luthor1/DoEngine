#pragma once

#include "../Base.h"
#include <vector>
#include <string>
#include <memory>

namespace DoEngine {

    struct FileData {
        std::vector<uint8_t> Data;
        size_t Size = 0;
    };

    class IFileSystem {
    public:
        virtual ~IFileSystem() = default;

        virtual bool FileExists(const std::string& path) = 0;
        virtual bool ReadFile(const std::string& path, FileData& outData) = 0;
        virtual bool WriteFile(const std::string& path, const void* data, size_t size) = 0;
    };

    class VFS {
    public:
        static void Initialize(const std::string& engineRoot);
        static void Shutdown();

        // Mount a physical directory or a pack file
        static void Mount(const std::string& virtualPath, std::unique_ptr<IFileSystem> fs);

        // Convenience: Mount a physical path directly
        static void MountPath(const std::string& virtualPath, const std::string& physicalPath);

        // Global file access
        static bool FileExists(const std::string& path);
        static bool ReadFile(const std::string& path, FileData& outData);
        
    private:
        struct MountPoint {
            std::string VirtualRoot;
            std::unique_ptr<IFileSystem> FileSystem;
        };

        static std::vector<MountPoint> s_MountPoints;
    };

}
