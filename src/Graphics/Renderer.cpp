#include "Renderer.h"
#include "../Core/Base.h"
#include <iostream>

namespace DoEngine {

    Renderer::Renderer(RHIDevice* rhi) : m_RHI(rhi) {
        DO_LOG("Renderer system initialized.");
    }

    Renderer::~Renderer() {
        DO_LOG("Renderer system shut down.");
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
