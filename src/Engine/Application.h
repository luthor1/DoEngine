#pragma once
#include "../Core/Base.h"
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <vector>

#include "../Core/Window.h"
#include "../Graphics/RHI.h"
#include "../Graphics/GraphicsTypes.h"
#include "../ECS/Scene.h"
#include "../Assets/AssetRegistry.h"
#include "../Audio/AudioSystem.h"
#include "../Physics/PhysicsSystem.h"
#include "../Scripting/ScriptEngine.h"
#include "../Core/JobSystem/JobSystem.h"
#include "../Core/VFS/VFS.h"
#include "../Graphics/Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace DoEngine {

    enum class EngineState { Hub, Editor };
    
    class Renderer;
    class Scene;
    class Editor;

    class Application {
    public:
        Application();
        virtual ~Application();

        void Run();
        void Shutdown();
        void Stop() { m_Running = false; }

        void OpenProject(const std::string& path);

    private:
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<RHIDevice> m_RHIDevice;
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<Scene> m_ActiveScene;
        std::unique_ptr<Editor> m_Editor;
        
        EngineState m_State = EngineState::Hub;
        bool m_Running = true;

        float m_LastFrameTime = 0.0f;
        float m_DeltaTime = 0.0f;

        // Resource handles (Abstracted)
        BufferHandle m_TriangleVB;
        BufferHandle m_UniformBuffer;
        BindingLayoutHandle m_BindingLayout;
        BindingSetHandle m_BindingSet;
        PipelineHandle m_TrianglePipeline;
        
        UniformData m_UniformData;
        bool m_ResourcesUploaded = false;
    };

} // namespace DoEngine
