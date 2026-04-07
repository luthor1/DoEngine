#pragma once

#include "../Core/Base.h"
#include <memory>
#include <string>

#include "../Core/VFS/VFS.h"
#include "../Core/JobSystem/JobSystem.h"
#include "../Graphics/RHI.h"
#include "../ECS/Scene.h"
#include "../Scripting/ScriptEngine.h"
#include "../Physics/PhysicsSystem.h"
#include "../Audio/AudioSystem.h"
#include "../Assets/AssetRegistry.h"

namespace DoEngine {
    class Application {
    public:
        Application();
        virtual ~Application();

        // Starts the engine
        void Run();
        
        // Shuts down the engine
        void Shutdown();

    private:
        bool m_Running = true;
        
        // Systems
        std::unique_ptr<RHIDevice> m_RHIDevice;
        std::unique_ptr<Scene> m_ActiveScene;
        std::unique_ptr<AssetRegistry> m_AssetRegistry;
    };
}
