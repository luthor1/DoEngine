#include "Renderer.h"
#include "../Core/Base.h"
#include "../ECS/Scene.h"
#include "../ECS/Components.h"
#include <iostream>

namespace DoEngine {

    Renderer::Renderer(RHIDevice* rhi) : m_RHI(rhi) {
        DO_CORE_INFO("Renderer system initialized.");
    }

    Renderer::~Renderer() {
        DO_CORE_INFO("Renderer system shut down.");
    }

    void Renderer::BeginFrame() {
        m_RHI->BeginFrame();
    }

    void Renderer::EndFrame() {
        m_RHI->EndFrame();
    }

    void Renderer::Clear(float r, float g, float b, float a) {
        m_RHI->Clear(r, g, b, a);
    }

    void Renderer::RenderScene(Scene* scene) {
        auto view = scene->View<MeshComponent, TransformComponent>();

        for (auto entity : view) {
            auto& mesh = view.get<MeshComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);

            if (mesh.Pipeline && mesh.VertexBuffer) {
                m_RHI->BindPipeline(mesh.Pipeline, mesh.VertexBuffer);
                
                // If the entity has a material, bind its resources (textures, samplers, etc)
                if (scene->GetRegistry().all_of<MaterialComponent>(entity)) {
                    auto& material = scene->Get<MaterialComponent>(entity);
                    if (material.BindingSet) {
                        m_RHI->BindBindingSet(1, material.BindingSet); // Slot 1 for materials
                    }
                }

                // Note: Slot 0 is usually for global uniforms (View/Proj)
                // For now, let's assume global uniforms are set up in Application.cpp
                
                m_RHI->Draw(mesh.VertexCount);
            }
        }
    }

    PipelineHandle Renderer::CreatePipeline(const PipelineDesc& desc) {
        return m_RHI->CreateGraphicsPipeline(desc);
    }

    void Renderer::BindPipeline(PipelineHandle pipeline, BufferHandle vb) {
        m_RHI->BindPipeline(pipeline, vb);
    }

    void Renderer::BindBindingSet(uint32_t slot, BindingSetHandle set) {
        m_RHI->BindBindingSet(slot, set);
    }

    void Renderer::SubmitDraw(uint32_t vertexCount, uint32_t instanceCount) {
        m_RHI->Draw(vertexCount, instanceCount);
    }

}
