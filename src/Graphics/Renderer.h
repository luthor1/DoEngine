#pragma once

#include "RHI.h"
#include <memory>
#include <string>

namespace DoEngine {

    class Renderer {
    public:
        Renderer(RHIDevice* rhi);
        ~Renderer();

        void BeginFrame();
        void EndFrame();

        // High-level Draw commands
        void Clear(float r, float g, float b, float a);
        void DrawTriangle(); // Temporary for demo

        // Pipeline management
        PipelineHandle CreatePipeline(const PipelineDesc& desc);
        void BindPipeline(PipelineHandle pipeline, BufferHandle vb = nullptr);
        void BindBindingSet(uint32_t slot, BindingSetHandle set);
        
        // Resource submission
        void SubmitDraw(uint32_t vertexCount, uint32_t instanceCount = 1);

    private:
        RHIDevice* m_RHI;
    };

}
