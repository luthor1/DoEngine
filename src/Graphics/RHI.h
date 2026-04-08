#pragma once

#include "../Core/Base.h"
#include <memory>
#include <vector>
#include <string>

namespace DoEngine {

    enum class GraphicsAPI {
        Vulkan,
        D3D12
    };

    // --- RHI Opaque Handles ---
    struct IResource { virtual ~IResource() = default; };
    using BufferHandle = std::shared_ptr<IResource>;
    using TextureHandle = std::shared_ptr<IResource>;
    using ShaderHandle = std::shared_ptr<IResource>;
    using InputLayoutHandle = std::shared_ptr<IResource>;
    using BindingLayoutHandle = std::shared_ptr<IResource>;
    using BindingSetHandle = std::shared_ptr<IResource>;
    using PipelineHandle = std::shared_ptr<IResource>;

    // --- RHI Descriptors ---
    struct BufferDesc {
        uint64_t byteSize = 0;
        bool isVertexBuffer = false;
        bool isIndexBuffer = false;
        bool isConstantBuffer = false;
        const char* debugName = "";
    };

    struct VertexAttribute {
        const char* name = "";
        uint32_t offset = 0;
        uint32_t stride = 0;
    };

    struct PipelineDesc {
        ShaderHandle vertexShader;
        ShaderHandle pixelShader;
        InputLayoutHandle inputLayout;
        BindingLayoutHandle bindingLayout;
    };

    class RHIDevice {
    public:
        RHIDevice();
        ~RHIDevice();

        bool Initialize(GraphicsAPI api, void* windowHandle);
        void Shutdown();

        // Frame Management
        void BeginFrame();
        void EndFrame();
        void Clear(float r, float g, float b, float a);

        // Resource Creation
        BufferHandle CreateBuffer(const BufferDesc& desc);
        ShaderHandle CreateShader(const char* entryPoint, const char* stage, const void* binary, size_t size);
        InputLayoutHandle CreateInputLayout(const VertexAttribute* attributes, uint32_t count);
        BindingLayoutHandle CreateBindingLayout(uint32_t constantBufferSlot);
        BindingSetHandle CreateBindingSet(BindingLayoutHandle layout, BufferHandle constantBuffer);
        PipelineHandle CreateGraphicsPipeline(const PipelineDesc& desc);

        // Commands
        void BindPipeline(PipelineHandle pipeline, BufferHandle vb = nullptr);
        void BindBindingSet(uint32_t slot, BindingSetHandle set);
        void WriteBuffer(BufferHandle buffer, const void* data, size_t size);
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1);

        GraphicsAPI GetCurrentAPI() const { return m_CurrentAPI; }

    private:
        bool InitVulkan(void* windowHandle);
        bool InitD3D12(void* windowHandle);

    private:
        GraphicsAPI m_CurrentAPI;
        
        struct BackendData;
        std::unique_ptr<BackendData> m_BackendData;
    };
}
