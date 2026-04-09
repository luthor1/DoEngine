#include "AssetImporter.h"
#include "../Core/VFS/VFS.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cstdint>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../thirdparty/stb/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace DoEngine {

    bool AssetImporter::ImportFBX(const std::string& inputPath, const std::string& outputPath) {
        std::cout << "[INFO] Importing FBX (Placeholder): " << inputPath << std::endl;
        return true;
    }

    bool AssetImporter::ImportPNG(const std::string& inputPath, const std::string& outputPath) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* pixels = stbi_load(inputPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        
        if (!pixels) {
            std::cerr << "[ERROR] Failed to load PNG: " << inputPath << std::endl;
            return false;
        }

        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            stbi_image_free(pixels);
            return false;
        }

        // Header: Width, Height, Channels (always 4 for RGBA)
        uint32_t w = (uint32_t)width;
        uint32_t h = (uint32_t)height;
        uint32_t c = 4;
        file.write((char*)&w, sizeof(uint32_t));
        file.write((char*)&h, sizeof(uint32_t));
        file.write((char*)&c, sizeof(uint32_t));
        
        // Pixel Data
        file.write((char*)pixels, width * height * 4);
        file.close();

        stbi_image_free(pixels);
        std::cout << "[INFO] Imported PNG to: " << outputPath << " (" << width << "x" << height << ")" << std::endl;
        return true;
    }

    // Simple OBJ Loader
    static bool ImportOBJ(const std::string& inputPath, MeshData& outData) {
        std::ifstream file(inputPath);
        if (!file.is_open()) return false;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                glm::vec3 p;
                ss >> p.x >> p.y >> p.z;
                positions.push_back(p);
            } else if (prefix == "vt") {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                uvs.push_back(uv);
            } else if (prefix == "vn") {
                glm::vec3 n;
                ss >> n.x >> n.y >> n.z;
                normals.push_back(n);
            } else if (prefix == "f") {
                for (int i = 0; i < 3; i++) {
                    std::string vertexStr;
                    ss >> vertexStr;
                    std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                    std::stringstream vss(vertexStr);
                    uint32_t vi, ti, ni;
                    vss >> vi >> ti >> ni;

                    Vertex v;
                    v.Position = positions[vi - 1];
                    v.TexCoord = ti > 0 ? uvs[ti - 1] : glm::vec2(0.0f);
                    v.Normal = ni > 0 ? normals[ni - 1] : glm::vec3(0.0f);
                    v.Color = glm::vec4(1.0f);

                    outData.Vertices.push_back(v);
                    outData.Indices.push_back((uint32_t)outData.Indices.size());
                }
            }
        }
        return true;
    }

    bool AssetImporter::LoadMeshBinary(const std::string& path, MeshData& outData) {
        if (path.ends_with(".obj")) {
            return ImportOBJ(path, outData);
        }

        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Could not open mesh binary: " << path << std::endl;
            return false;
        }

        uint32_t vertexCount = 0;
        file.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
        if (vertexCount > 0) {
            outData.Vertices.resize(vertexCount);
            file.read(reinterpret_cast<char*>(outData.Vertices.data()), vertexCount * sizeof(Vertex));
        }

        uint32_t indexCount = 0;
        file.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));
        if (indexCount > 0) {
            outData.Indices.resize(indexCount);
            file.read(reinterpret_cast<char*>(outData.Indices.data()), indexCount * sizeof(uint32_t));
        }

        return true;
    }

    bool AssetImporter::LoadTextureBinary(const std::string& path, TextureData& outData) {
        // Point 5: Add direct PNG loading support
        if (path.ends_with(".png") || path.ends_with(".jpg") || path.ends_with(".tga")) {
            int width, height, channels;
            unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            
            if (!pixels) {
                std::cerr << "[ERROR] Failed to load image: " << path << std::endl;
                return false;
            }

            outData.Width = (uint32_t)width;
            outData.Height = (uint32_t)height;
            outData.Channels = 4;
            outData.Pixels.assign(pixels, pixels + (static_cast<size_t>(width) * height * 4));
            stbi_image_free(pixels);
            return true;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) return false;

        file.read(reinterpret_cast<char*>(&outData.Width), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&outData.Height), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&outData.Channels), sizeof(uint32_t));

        size_t size = static_cast<size_t>(outData.Width) * outData.Height * outData.Channels;
        if (size > 0) {
            outData.Pixels.resize(size);
            file.read(reinterpret_cast<char*>(outData.Pixels.data()), size);
        }

        return true;
    }

    bool AssetImporter::LoadShaderBinary(const std::string& path, ShaderData& outData) {
        FileData vfsData;
        if (VFS::ReadFile(path, vfsData)) {
            outData.Bytecode.assign(vfsData.Data.begin(), vfsData.Data.end());
            
            if (path.find(".vs") != std::string::npos) outData.Stage = "vs";
            else if (path.find(".ps") != std::string::npos) outData.Stage = "ps";
            
            return true;
        }

        // Fallback to raw disk access if VFS fails (for non-virtual paths)
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return false;

        size_t size = (size_t)file.tellg();
        outData.Bytecode.resize(size);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(outData.Bytecode.data()), size);

        if (path.find(".vs") != std::string::npos) outData.Stage = "vs";
        else if (path.find(".ps") != std::string::npos) outData.Stage = "ps";

        return true;
    }
}
