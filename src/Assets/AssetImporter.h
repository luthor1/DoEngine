#pragma once

#include "../Core/Base.h"
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

#include "../Graphics/GraphicsTypes.h" // For Vertex struct

namespace DoEngine {

    struct MeshData {
        std::vector<Vertex> Vertices;
        std::vector<uint32> Indices;
    };

    struct TextureData {
        uint32 Width, Height, Channels;
        std::vector<uint8_t> Pixels;
    };

    struct ShaderData {
        std::vector<char> Bytecode;
        std::string Stage; // "vs" or "ps"
    };

    class AssetImporter {
    public:
        // Main import function (transforms Ham to Binary)
        static bool ImportFBX(const std::string& inputPath, const std::string& outputPath);
        static bool ImportPNG(const std::string& inputPath, const std::string& outputPath);

        // Helper to load binary files
        static bool LoadMeshBinary(const std::string& path, MeshData& outData);
        static bool LoadTextureBinary(const std::string& path, TextureData& outData);
        static bool LoadShaderBinary(const std::string& path, ShaderData& outData);
    };

}
