#include "Application.h"
#include <iostream>

#include "../Core/VFS/VFS.h"
#include "../Core/JobSystem/JobSystem.h"
#include "../Graphics/RHI.h"
#include "../ECS/Scene.h"
#include "../Scripting/ScriptEngine.h"
#include "../Physics/PhysicsSystem.h"
#include "../Audio/AudioSystem.h"

namespace DoEngine {

    Application::Application() {
        std::cout << "[INFO] Initializing DoEngine Application..." << std::endl;
        
        // 1. Initialize Core Systems
        VFS::Initialize();
        JobSystem::Initialize();
        
        // 2. Initialize Subsystems (Placeholders/Taslak)
        m_RHIDevice = std::make_unique<RHIDevice>();
        m_RHIDevice->Initialize(nullptr); // window placeholder
        
        ScriptEngine::Initialize();
        PhysicsSystem::Initialize();
        AudioSystem::Initialize();
        AssetRegistry::Initialize();
        
        m_ActiveScene = std::make_unique<Scene>();
    }

    Application::~Application() {
        std::cout << "[INFO] Shutting down DoEngine Application..." << std::endl;
        
        m_ActiveScene.reset();
        
        AssetRegistry::Shutdown();
        AudioSystem::Shutdown();
        PhysicsSystem::Shutdown();
        ScriptEngine::Shutdown();
        m_RHIDevice->Shutdown();
        
        JobSystem::Shutdown();
        VFS::Shutdown();
    }

    void Application::Run() {
        std::cout << "[INFO] Engine loop started." << std::endl;
        
        float deltaTime = 0.016f; // Placeholder for 60fps
        
        while (m_Running) {
            // 1. Update Core
            
            // 2. Update Subsystems
            PhysicsSystem::OnUpdate(deltaTime);
            ScriptEngine::OnUpdate(deltaTime);
            m_ActiveScene->OnUpdate(deltaTime);
            AudioSystem::OnUpdate(deltaTime);
            
            // 3. Render
            m_RHIDevice->BeginFrame();
            // Render scene here...
            m_RHIDevice->EndFrame();

            // Placeholder: Check for window close or escape key
            // For now, let's just make it run once to test the base logic.
            m_Running = false;
        }
        
        Shutdown();
    }

    void Application::Shutdown() {
        std::cout << "[INFO] Cleaning up resources..." << std::endl;
    }

}
