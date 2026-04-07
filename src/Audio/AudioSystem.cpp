#include "AudioSystem.h"
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace DoEngine {

    ma_engine* AudioSystem::s_Engine = nullptr;

    void AudioSystem::Initialize() {
        std::cout << "[INFO] Initializing miniaudio engine..." << std::endl;
    }

    void AudioSystem::Shutdown() {
        if (s_Engine) {
            // ma_engine_uninit(s_Engine);
            delete s_Engine;
            s_Engine = nullptr;
        }
        std::cout << "[INFO] Audio System shut down." << std::endl;
    }

    void AudioSystem::PlayAudio(const std::string& path) {
        std::cout << "[TRACE] Playing audio: " << path << std::endl;
    }

    void AudioSystem::OnUpdate(float deltaTime) {
    }

}
