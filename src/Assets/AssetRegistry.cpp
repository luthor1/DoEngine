#include "AssetRegistry.h"
#include <iostream>

namespace DoEngine {

    std::unordered_map<std::string, std::shared_ptr<Asset>> AssetRegistry::s_AssetCache;

    void AssetRegistry::Initialize() {
        std::cout << "[INFO] Asset Registry initialized." << std::endl;
    }

    void AssetRegistry::Shutdown() {
        s_AssetCache.clear();
        std::cout << "[INFO] Asset Registry shut down." << std::endl;
    }

    std::shared_ptr<MeshAsset> AssetRegistry::GetMesh(const std::string& path) {
        if (s_AssetCache.find(path) != s_AssetCache.end()) {
            return std::dynamic_pointer_cast<MeshAsset>(s_AssetCache[path]);
        }

        // Create Mesh asset
        MeshData data;
        auto mesh = std::make_shared<MeshAsset>(path, std::move(data));
        s_AssetCache[path] = mesh;
        return mesh;
    }

    std::shared_ptr<TextureAsset> AssetRegistry::GetTexture(const std::string& path) {
        if (s_AssetCache.find(path) != s_AssetCache.end()) {
            return std::dynamic_pointer_cast<TextureAsset>(s_AssetCache[path]);
        }

        // Create Texture asset
        TextureData data;
        auto texture = std::make_shared<TextureAsset>(path, std::move(data));
        s_AssetCache[path] = texture;
        return texture;
    }

}
