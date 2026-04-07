#pragma once

#include "../Core/Base.h"
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace DoEngine {

    struct MeshData {
        // Vertex and index data...
        std::vector<float> Vertices;
        std::vector<uint32> Indices;
    };

    struct TextureData {
        // Pixel data and metadata...
        uint32 Width, Height, Channels;
        std::vector<uint8_t> Pixels;
    };

    class AssetImporter {
    public:
        // Main import function (transforms Ham to Binary)
        static bool ImportFBX(const std::string& inputPath, const std::string& outputPath);
        static bool ImportPNG(const std::string& inputPath, const std::string& outputPath);

        // Helper to load binary files
        static bool LoadMeshBinary(const std::string& path, MeshData& outData);
        static bool LoadTextureBinary(const std::string& path, TextureData& outData);
    };

}
