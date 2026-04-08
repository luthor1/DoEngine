#include "Application.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <vector>
#include <cmath>

namespace DoEngine {

    Application::Application() {
        // Configure spdlog for the engine
        spdlog::set_pattern("[%T.%e] [%^%l%$] [thread %t] %v");
        DO_CORE_INFO("Initializing DoEngine Application...");
        
        // 1. Initialize Core Systems
        VFS::Initialize();
        JobSystem::Initialize();
        
        // 2. Initialize Window
        m_Window = std::make_unique<Window>(WindowProps("DoEngine - Standard Core Libs", 1280, 720));
        
        // 3. Initialize RHI & Renderer
        m_RHIDevice = std::make_unique<RHIDevice>();
        if (!m_RHIDevice->Initialize(GraphicsAPI::Vulkan, m_Window->GetNativeWindow())) {
            DO_CORE_ERROR("Failed to initialize RHI!");
            m_Running = false;
        }
        m_Renderer = std::make_unique<Renderer>(m_RHIDevice.get());
        
        ScriptEngine::Initialize();
        PhysicsSystem::Initialize();
        AudioSystem::Initialize();
        AssetRegistry::Initialize();
        
        m_ActiveScene = std::make_unique<Scene>();

        // 4. Create Resources via Renderer/AssetRegistry
        BufferDesc vbDesc;
        vbDesc.byteSize = sizeof(Vertex) * 3;
        vbDesc.isVertexBuffer = true;
        vbDesc.debugName = "TriangleVB";
        m_TriangleVB = m_RHIDevice->CreateBuffer(vbDesc);

        BufferDesc ubDesc;
        ubDesc.byteSize = sizeof(UniformData);
        ubDesc.isConstantBuffer = true;
        ubDesc.debugName = "TriangleUB";
        m_UniformBuffer = m_RHIDevice->CreateBuffer(ubDesc);

        VertexAttribute attributes[] = {
            { "POSITION", 0, sizeof(Vertex) },
            { "COLOR",    sizeof(glm::vec3), sizeof(Vertex) }
        };
        auto inputLayout = m_RHIDevice->CreateInputLayout(attributes, 2);
        m_BindingLayout = m_RHIDevice->CreateBindingLayout(0);
        m_BindingSet = m_RHIDevice->CreateBindingSet(m_BindingLayout, m_UniformBuffer);

        // 5. Load Shaders & Create Pipeline
        auto vsAsset = AssetRegistry::GetShader("shaders/Triangle.vs.spv");
        auto psAsset = AssetRegistry::GetShader("shaders/Triangle.ps.spv");

        if (vsAsset && psAsset) {
            auto& vsData = vsAsset->GetData();
            auto& psData = psAsset->GetData();

            auto vsHandle = m_RHIDevice->CreateShader("main", vsData.Stage.c_str(), vsData.Bytecode.data(), vsData.Bytecode.size());
            auto psHandle = m_RHIDevice->CreateShader("main", psData.Stage.c_str(), psData.Bytecode.data(), psData.Bytecode.size());

            PipelineDesc pDesc;
            pDesc.vertexShader = vsHandle;
            pDesc.pixelShader = psHandle;
            pDesc.inputLayout = inputLayout;
            pDesc.bindingLayout = m_BindingLayout;
            m_TrianglePipeline = m_Renderer->CreatePipeline(pDesc);
            
            DO_CORE_INFO("Graphics Pipeline created successfully with shaders from AssetRegistry.");
        } else {
            DO_CORE_ERROR("Failed to load shaders! Check shaders/ directory.");
        }
    }

    Application::~Application() {
        Shutdown();
    }

    void Application::Run() {
        DO_CORE_INFO("Engine loop started.");
        
        while (m_Running && !m_Window->ShouldClose()) {
            m_Window->OnUpdate();
            
            float deltaTime = 0.016f; // Placeholder
            PhysicsSystem::OnUpdate(deltaTime);
            m_ActiveScene->OnUpdate(deltaTime);
            
            // Render
            m_Renderer->BeginFrame();
            
            if (!m_ResourcesUploaded) {
                Vertex vertices[] = {
                    { { 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                    { { 0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                    { {-0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
                };
                m_RHIDevice->WriteBuffer(m_TriangleVB, vertices, sizeof(vertices));
                m_ResourcesUploaded = true;
            }

            m_Renderer->Clear(0.15f, 0.15f, 0.18f, 1.0f);
            
            if (m_TrianglePipeline) {
                m_Renderer->BindPipeline(m_TrianglePipeline, m_TriangleVB);
                
                // Using GLM for modern matrix math
                float time = (float)glfwGetTime();
                glm::mat4 model = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 view  = glm::lookAt(glm::vec3(0,0,2), glm::vec3(0,0,0), glm::vec3(0,1,0));
                glm::mat4 proj  = glm::perspective(glm::radians(45.0f), 1280.0f/720.0f, 0.1f, 10.0f);
                // Vulkan clip space has inverted Y and half-Z
                proj[1][1] *= -1; 
                
                m_UniformData.MVP = proj * view * model;
                m_RHIDevice->WriteBuffer(m_UniformBuffer, &m_UniformData, sizeof(UniformData));
                
                m_Renderer->BindBindingSet(0, m_BindingSet);
                m_Renderer->SubmitDraw(3);
            }
            
            m_Renderer->EndFrame();
        }
    }

    void Application::Shutdown() {
        DO_CORE_INFO("Shutting down engine systems...");
        
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
