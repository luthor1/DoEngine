#include "../Core/Base.h"
#include <memory>
#include <string>
#include <vector>

#include "../Core/Window.h"
#include "../Graphics/RHI.h"
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

    struct Vertex {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    struct UniformData {
        glm::mat4 MVP;
    };

    class Application {
    public:
        Application();
        virtual ~Application();

        void Run();
        void Shutdown();
        void Stop() { m_Running = false; }

    private:
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<RHIDevice> m_RHIDevice;
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<Scene> m_ActiveScene;
        bool m_Running = true;

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
