#include "VFS.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace DoEngine {

    std::vector<VFS::MountPoint> VFS::s_MountPoints;

    class PhysicalFileSystem : public IFileSystem {
    public:
        PhysicalFileSystem(const std::string& physicalRoot) : m_Root(physicalRoot) {}

        bool FileExists(const std::string& path) override {
            std::ifstream f(m_Root + "/" + path);
            return f.good();
        }

        bool ReadFile(const std::string& path, FileData& outData) override {
            std::ifstream file(m_Root + "/" + path, std::ios::binary | std::ios::ate);
            if (!file.is_open()) return false;

            outData.Size = file.tellg();
            outData.Data.resize(outData.Size);
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(outData.Data.data()), outData.Size);
            file.close();

            return true;
        }

        bool WriteFile(const std::string& path, const void* data, size_t size) override {
            std::ofstream file(m_Root + "/" + path, std::ios::binary);
            if (!file.is_open()) return false;

            file.write(reinterpret_cast<const char*>(data), size);
            file.close();
            return true;
        }

    private:
        std::string m_Root;
    };

    void VFS::Initialize(const std::string& engineRoot) {
        // Mount the engine root to engine://
        Mount("engine://", std::make_unique<PhysicalFileSystem>(engineRoot));
        
        std::cout << "[INFO] VFS: Engine Root mounted at " << engineRoot << std::endl;
        std::cout << "[INFO] Virtual File System initialized." << std::endl;
    }

    void VFS::Shutdown() {
        s_MountPoints.clear();
    }

    void VFS::Mount(const std::string& virtualPath, std::unique_ptr<IFileSystem> fs) {
        s_MountPoints.push_back({ virtualPath, std::move(fs) });
        std::sort(s_MountPoints.begin(), s_MountPoints.end(), [](const MountPoint& a, const MountPoint& b) {
            return a.VirtualRoot.length() > b.VirtualRoot.length();
        });
    }

    bool VFS::FileExists(const std::string& path) {
        for (auto& mp : s_MountPoints) {
            if (path.find(mp.VirtualRoot) == 0) {
                std::string relativePath = path.substr(mp.VirtualRoot.length());
                if (mp.FileSystem->FileExists(relativePath)) return true;
            }
        }
        return false;
    }

    bool VFS::ReadFile(const std::string& path, FileData& outData) {
        for (auto& mp : s_MountPoints) {
            if (path.find(mp.VirtualRoot) == 0) {
                std::string relativePath = path.substr(mp.VirtualRoot.length());
                if (mp.FileSystem->ReadFile(relativePath, outData)) return true;
            }
        }
        return false;
    }

}
