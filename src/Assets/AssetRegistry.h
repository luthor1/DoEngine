#pragma once

#include "../Core/Base.h"
#include "AssetImporter.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>

namespace DoEngine {

    class Asset {
    public:
        Asset(const std::string& path) : m_Path(path) {}
        virtual ~Asset() = default;

        const std::string& GetPath() const { return m_Path; }

    protected:
        std::string m_Path;
    };

    class MeshAsset : public Asset {
    public:
        MeshAsset(const std::string& path, MeshData data) : Asset(path), m_Data(std::move(data)) {}
        const MeshData& GetData() const { return m_Data; }

    private:
        MeshData m_Data;
    };

    class TextureAsset : public Asset {
    public:
        TextureAsset(const std::string& path, TextureData data) : Asset(path), m_Data(std::move(data)) {}
        const TextureData& GetData() const { return m_Data; }

    private:
        TextureData m_Data;
    };

    class ShaderAsset : public Asset {
    public:
        ShaderAsset(const std::string& path, ShaderData data) : Asset(path), m_Data(std::move(data)) {}
        const ShaderData& GetData() const { return m_Data; }

    private:
        ShaderData m_Data;
    };

    class AssetRegistry {
    public:
        static void Initialize();
        static void Shutdown();

        // Get or load a mesh
        static std::shared_ptr<MeshAsset> GetMesh(const std::string& path);
        
        // Get or load a texture
        static std::shared_ptr<TextureAsset> GetTexture(const std::string& path);

        // Get or load a shader
        static std::shared_ptr<ShaderAsset> GetShader(const std::string& path);

    private:
        // Cache of loaded assets
        static std::unordered_map<std::string, std::shared_ptr<Asset>> s_AssetCache;
    };

}
