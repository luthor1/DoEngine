#pragma once

#include "../Core/Base.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

// NVRHI'nin temel tiplerini kullanabilmek için ekliyoruz
#include <nvrhi/nvrhi.h>

// Raw Vulkan handle tipleri (forward declare ile include kaçınıyoruz)
typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;
typedef struct VkRenderPass_T* VkRenderPass;

namespace DoEngine {

enum class GraphicsAPI { Vulkan, D3D12 };

// --- RHI Opaque Handles ---
struct IResource {
  virtual ~IResource() = default;
};

template <typename T> struct ResourceWrapper : public IResource {
  nvrhi::RefCountPtr<T> handle;
  ResourceWrapper(T *p) : handle(p) {}
};
using BufferHandle = std::shared_ptr<IResource>;
using TextureHandle = std::shared_ptr<IResource>;
using SamplerHandle = std::shared_ptr<IResource>;
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
  const char *debugName = "";
};

struct VertexAttribute {
  const char *name = "";
  uint32_t offset = 0;
  uint32_t stride = 0;
};

struct TextureDesc {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  uint32_t mipLevels = 1;
  const char *debugName = "";
};

enum class BindingType { ConstantBuffer, Texture, Sampler };

struct BindingLayoutItem {
  uint32_t slot;
  BindingType type;
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

  bool Initialize(GraphicsAPI api, void *windowHandle);
  void Shutdown();

  // GPU Senkronizasyonu
  void WaitForIdle();

  // NVRHI Command & Device Management
  nvrhi::IDevice *GetDevice() const;
  nvrhi::CommandListHandle CreateCommandList();
  nvrhi::CommandListHandle GetCurrentCommandList() const;
  void ExecuteCommandList(nvrhi::CommandListHandle cmdList);
  nvrhi::IFramebuffer* GetCurrentFramebuffer() const;

  // Frame Management
  void BeginFrame();
  void EndFrame();
  void Clear(float r, float g, float b, float a);

  // Resource Creation
  BufferHandle CreateBuffer(const BufferDesc &desc);
  TextureHandle CreateTexture(const TextureDesc &desc);
  SamplerHandle CreateSampler();
  ShaderHandle CreateShader(const char *entryPoint, const char *stage,
                            const void *binary, size_t size);
  InputLayoutHandle CreateInputLayout(const VertexAttribute *attributes,
                                      uint32_t count);
  BindingLayoutHandle CreateBindingLayout(const BindingLayoutItem *items,
                                          uint32_t count);
  BindingSetHandle CreateBindingSet(
      BindingLayoutHandle layout,
      const std::vector<std::pair<uint32_t, std::shared_ptr<IResource>>>
          &resources);
  PipelineHandle CreateGraphicsPipeline(const PipelineDesc &desc);

  // Commands
  void BindPipeline(PipelineHandle pipeline, BufferHandle vb = nullptr);
  void BindBindingSet(uint32_t slot, BindingSetHandle set);
  void WriteBuffer(BufferHandle buffer, const void *data, size_t size);
  void WriteTexture(TextureHandle texture, const void *data, size_t size);
  void Draw(uint32_t vertexCount, uint32_t instanceCount = 1);

  GraphicsAPI GetCurrentAPI() const { return m_CurrentAPI; }

  // Raw Vulkan handle'larını imgui_impl_vulkan için expose et
  struct VulkanHandles {
    VkInstance       Instance       = nullptr;
    VkPhysicalDevice PhysicalDevice = nullptr;
    VkDevice         Device         = nullptr;
    VkQueue          GraphicsQueue  = nullptr;
    uint32_t         QueueFamily    = 0;
    uint32_t         MinImageCount  = 2;
    uint32_t         ImageCount     = 3;
  };
  VulkanHandles GetVulkanHandles() const;

private:
  bool InitVulkan(void *windowHandle);
  bool InitD3D12(void *windowHandle);

private:
  GraphicsAPI m_CurrentAPI;

  struct BackendData;
  std::unique_ptr<BackendData> m_BackendData;
};
} // namespace DoEngine