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

        // Load Engine-Internal Shaders for the Renderer/Editor
        auto vsAsset = AssetRegistry::GetShader("engine://shaders/Standard.vs.spv");
        auto psAsset = AssetRegistry::GetShader("engine://shaders/Standard.ps.spv");

        if (vsAsset && psAsset) {
            auto vsHandle = m_RHIDevice->CreateShader("main", "vs", vsAsset->GetData().Bytecode.data(), vsAsset->GetData().Bytecode.size());
            auto psHandle = m_RHIDevice->CreateShader("main", "ps", psAsset->GetData().Bytecode.data(), psAsset->GetData().Bytecode.size());

            // Prepare Global Layouts
            VertexAttribute attributes[] = {
                { "POSITION", 0,                          sizeof(Vertex) },
                { "COLOR",    offsetof(Vertex, Color),    sizeof(Vertex) },
                { "NORMAL",   offsetof(Vertex, Normal),   sizeof(Vertex) },
                { "TEXCOORD", offsetof(Vertex, TexCoord), sizeof(Vertex) }
            };
            auto inputLayout = m_RHIDevice->CreateInputLayout(attributes, 4);
            
            BindingLayoutItem globalItems[] = { { 0, BindingType::ConstantBuffer } };
            m_BindingLayout = m_RHIDevice->CreateBindingLayout(globalItems, 1);

            if (vsHandle && psHandle && inputLayout && m_BindingLayout) {
                PipelineDesc pDesc;
                pDesc.vertexShader = vsHandle;
                pDesc.pixelShader = psHandle;
                pDesc.inputLayout = inputLayout;
                pDesc.bindingLayout = m_BindingLayout;
                m_TrianglePipeline = m_Renderer->CreatePipeline(pDesc);
                
                // Uniforms
                BufferDesc ubDesc;
                ubDesc.byteSize = sizeof(UniformData);
                ubDesc.isConstantBuffer = true;
                m_UniformBuffer = m_RHIDevice->CreateBuffer(ubDesc);
                
                if (m_UniformBuffer) {
                    std::vector<std::pair<uint32_t, std::shared_ptr<IResource>>> globalResources = { { 0, m_UniformBuffer } };
                    m_BindingSet = m_RHIDevice->CreateBindingSet(m_BindingLayout, globalResources);
                }
            }
        } else {
            DO_CORE_ERROR("Engine Internal Shaders (Standard) not found!");
        }

        DO_CORE_INFO("Application initialized successfully in Hub mode.");
    }

    Application::~Application() {
        Shutdown();
    }

    void Application::OpenProject(const std::string& path) {
        DO_CORE_INFO("Project System: Opening project at {}", path);
        // TODO: VFS Mount of the project path to "project://"
        m_State = EngineState::Editor;
    }

    void Application::Run() {
        DO_CORE_INFO("Application: Starting Main Loop...");
        m_LastFrameTime = (float)glfwGetTime();

        while (m_Running && !m_Window->ShouldClose()) {
            // 1. Girdileri Oku
            glfwPollEvents();

            float currentTime = (float)glfwGetTime();
            m_DeltaTime = currentTime - m_LastFrameTime;
            m_LastFrameTime = currentTime;

            // --- CRASH TRACE START ---
            DO_CORE_INFO("[TRACE] Starting Frame Operations...");

            m_Renderer->BeginFrame();
            DO_CORE_INFO("[TRACE] Renderer::BeginFrame success");

            m_Editor->BeginFrame();
            DO_CORE_INFO("[TRACE] Editor::BeginFrame success");

            DO_CORE_INFO("[TRACE] Attempting Renderer::Clear...");
            m_Renderer->Clear(0.12f, 0.12f, 0.14f, 1.0f);
            DO_CORE_INFO("[TRACE] Renderer::Clear success");
            
            if (m_State == EngineState::Hub) {
                DO_CORE_INFO("[TRACE] Attempting Editor::OnHubUpdate...");
                m_Editor->OnHubUpdate(this);
                DO_CORE_INFO("[TRACE] Editor::OnHubUpdate success");
            } else if (m_State == EngineState::Editor) {
                // Update Global Uniforms
                glm::mat4 model = glm::rotate(glm::mat4(1.0f), currentTime, glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 view  = glm::lookAt(glm::vec3(0,0,2), glm::vec3(0,0,0), glm::vec3(0,1,0));
                glm::mat4 proj  = glm::perspective(glm::radians(45.0f), 1280.0f/720.0f, 0.1f, 10.0f);
                proj[1][1] *= -1; 
                
                m_UniformData.MVP = proj * view * model;
                if (m_UniformBuffer) {
                    m_RHIDevice->WriteBuffer(m_UniformBuffer, &m_UniformData, sizeof(UniformData));
                }

                m_Renderer->RenderScene(m_ActiveScene.get());
                m_Editor->OnUpdate(m_ActiveScene.get());
            }

            DO_CORE_INFO("[TRACE] Attempting Editor::EndFrame...");
            m_Editor->EndFrame();
            DO_CORE_INFO("[TRACE] Editor::EndFrame success");

            m_Renderer->EndFrame();
            DO_CORE_INFO("[TRACE] Renderer::EndFrame success");
            
            // Sadece ilk karede log basıp kalabalığı önleyelim
            static bool firstFrame = true;
            if (firstFrame) {
                DO_CORE_INFO("First Frame completed successfully!");
                firstFrame = false;
            }
        }

        DO_CORE_INFO("Application: Exiting Main Loop. (m_Running: {}, ShouldClose: {})", m_Running, m_Window->ShouldClose());
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
