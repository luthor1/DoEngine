#include "Application.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Core/Base.h"
#include "../ECS/Components.h"
#include "../Editor/Editor.h"
#include "../Assets/AssetImporter.h"

#include "../Core/PathUtils.h"
#include "../Core/VFS/VFS.h"
#include <vector>

namespace DoEngine {

    Application::Application() {
        // Configure spdlog for the engine
        spdlog::set_pattern("[%T.%e] [%^%l%$] [thread %t] %v");
        DO_CORE_INFO("Initializing DoEngine Application...");
        
        // 1. Initialize Core Systems
        std::filesystem::path engineRoot = PathUtils::GetEngineRoot();
        DO_CORE_INFO("Engine Discovery: Root found at {}", engineRoot.string());
        VFS::Initialize(engineRoot.string());
        JobSystem::Initialize();
        
        // 2. Initialize Window
        m_Window = std::make_unique<Window>(WindowProps("DoEngine Hub", 1280, 720));
        
        // 3. Initialize RHI & Renderer
        m_RHIDevice = std::make_unique<RHIDevice>();
        if (!m_RHIDevice->Initialize(GraphicsAPI::Vulkan, m_Window->GetNativeWindow())) {
            DO_CORE_ERROR("Failed to initialize RHI!");
            m_Running = false;
        }
        m_Renderer = std::make_unique<Renderer>(m_RHIDevice.get());
        
        // Initialize UI / Editor
        m_Editor = std::make_unique<Editor>(m_RHIDevice.get(), (GLFWwindow*)m_Window->GetNativeWindow());

        // Initialize Other Engine Systems
        ScriptEngine::Initialize();
        PhysicsSystem::Initialize();
        AudioSystem::Initialize();
        AssetRegistry::Initialize();
        
        m_ActiveScene = std::make_unique<Scene>();

        // Pipeline kurulumu editor moduna girildiğinde yapılacak (lazy).
        // Hub modu yalnızca ImGui ile çalışır, 3D pipeline gerektirmez.
        DO_CORE_INFO("Application initialized successfully in Hub mode.");
    }

    Application::~Application() {
        Shutdown();
    }

    void Application::OpenProject(const std::string& path) {
        DO_CORE_INFO("Project System: Opening project at {}", path);
        
        ProjectInfo info;
        if (ProjectManager::OpenProject(path, info)) {
            // Proje VFS mount noktasına bağla
            VFS::MountPath("project://", path);
            
            // Content browser başlangıç yolu
            // Sahneyi sıfırla
            m_ActiveScene = std::make_unique<Scene>();
            
            m_State = EngineState::Editor;
            DO_CORE_INFO("Project opened: {}", info.Name);
        } else {
            DO_CORE_ERROR("Failed to open project at: {}", path);
        }
    }

    void Application::Run() {
        DO_CORE_INFO("Application: Starting Main Loop...");
        m_LastFrameTime = (float)glfwGetTime();

        while (m_Running && !m_Window->ShouldClose()) {
            glfwPollEvents();

            float currentTime = (float)glfwGetTime();
            m_DeltaTime = currentTime - m_LastFrameTime;
            m_LastFrameTime = currentTime;

            m_Renderer->BeginFrame();
            m_Editor->BeginFrame();
            m_Renderer->Clear(0.09f, 0.09f, 0.12f, 1.0f);

            if (m_State == EngineState::Hub) {
                m_Editor->OnHubUpdate(this);
            } else if (m_State == EngineState::Editor) {
                // Uniform güncelleme
                glm::mat4 model = glm::rotate(glm::mat4(1.0f), currentTime,
                                              glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 view  = glm::lookAt(glm::vec3(0,0,2),
                                              glm::vec3(0,0,0),
                                              glm::vec3(0,1,0));
                glm::mat4 proj  = glm::perspective(glm::radians(45.0f),
                    m_Editor->GetViewportWidth() > 0
                        ? m_Editor->GetViewportWidth() / m_Editor->GetViewportHeight()
                        : 1280.0f / 720.0f,
                    0.1f, 10.0f);
                proj[1][1] *= -1;

                m_UniformData.MVP = proj * view * model;
                if (m_UniformBuffer) {
                    m_RHIDevice->WriteBuffer(m_UniformBuffer, &m_UniformData, sizeof(UniformData));
                }

                m_Renderer->RenderScene(m_ActiveScene.get());
                m_Editor->OnUpdate(m_ActiveScene.get());
            }

            m_Editor->EndFrame();
            m_Renderer->EndFrame();
        }

        DO_CORE_INFO("Application: Exiting Main Loop.");
    }

    void Application::Shutdown() {
        DO_CORE_INFO("Shutting down engine systems...");
        
        if (m_Editor) m_Editor.reset();

        ScriptEngine::Shutdown();
        PhysicsSystem::Shutdown();
        AudioSystem::Shutdown();
        AssetRegistry::Shutdown();
        
        if (m_Renderer) m_Renderer.reset();
        if (m_RHIDevice) m_RHIDevice->Shutdown();
        
        JobSystem::Shutdown();
        VFS::Shutdown();
        
        m_Running = false;
    }
}
