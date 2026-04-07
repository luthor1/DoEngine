#pragma once

#include "../Core/Base.h"
#include <string>
#include <memory>

struct ma_engine; // Forward declaration

namespace DoEngine {

    class AudioSystem {
    public:
        static void Initialize();
        static void Shutdown();

        // Spatial and global audio methods
        static void PlayAudio(const std::string& path);
        static void OnUpdate(float deltaTime);

    private:
        static ma_engine* s_Engine;
    };

}
