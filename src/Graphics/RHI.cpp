#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <iostream>
#include <string>
#include <vector>

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <nvrhi/vulkan.h>
#include <vulkan/vulkan.hpp>

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "../Core/Base.h"
#include "RHI.h"

namespace DoEngine {


struct RHIDevice::BackendData {
  nvrhi::DeviceHandle m_Device;
  VkInstance Instance = VK_NULL_HANDLE;
  VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
  VkDevice Device = VK_NULL_HANDLE;
  VkQueue GraphicsQueue = VK_NULL_HANDLE;
  uint32_t GraphicsQueueFamily = 0;

  VkSurfaceKHR Surface = VK_NULL_HANDLE;
  VkSwapchainKHR SwapChain = VK_NULL_HANDLE;
  std::vector<nvrhi::TextureHandle> SwapChainTextures;
  std::vector<nvrhi::FramebufferHandle> SwapChainFramebuffers;
  uint32_t CurrentSwapChainIndex = 0;

  VkSemaphore ImageAvailableSemaphore = VK_NULL_HANDLE;
  VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;

  nvrhi::CommandListHandle m_CurrentCommandList;
  nvrhi::GraphicsState m_CurrentGraphicsState;
};

RHIDevice::RHIDevice() : m_CurrentAPI(GraphicsAPI::Vulkan) {
  m_BackendData = std::make_unique<BackendData>();
}

RHIDevice::~RHIDevice() { Shutdown(); }

bool RHIDevice::Initialize(GraphicsAPI api, void *windowHandle) {
  m_CurrentAPI = api;
  if (api == GraphicsAPI::Vulkan) {
    return InitVulkan(windowHandle);
  }
  return false;
}

bool RHIDevice::InitVulkan(void *windowHandle) {
  DO_LOG("Initializing RHI Vulkan Backend (Strict Abstraction)...");

  // 1. Instance
  VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.apiVersion = VK_API_VERSION_1_3;

  uint32_t glfwExtCount = 0;
  const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
  std::vector<const char *> exts(glfwExts, glfwExts + glfwExtCount);

  // Validation Layers
  const char* validationLayer = "VK_LAYER_KHRONOS_validation";
  std::vector<const char*> layers;
#ifdef _DEBUG
  layers.push_back(validationLayer);
  DO_LOG("Vulkan: Validation layers enabled.");
#endif

  VkInstanceCreateInfo instInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instInfo.pApplicationInfo = &appInfo;
  instInfo.enabledLayerCount = (uint32_t)layers.size();
  instInfo.ppEnabledLayerNames = layers.data();
  instInfo.enabledExtensionCount = static_cast<uint32_t>(exts.size());
  instInfo.ppEnabledExtensionNames = exts.data();

  if (vkCreateInstance(&instInfo, nullptr, &m_BackendData->Instance) !=
      VK_SUCCESS) {
    DO_CORE_ERROR("Failed to create Vulkan Instance!");
    return false;
  }

  // Instance için Vulkan Dispatcher Init
  VULKAN_HPP_DEFAULT_DISPATCHER.init(vk::Instance(m_BackendData->Instance),
                                     vkGetInstanceProcAddr);

  // 2. Surface & Physical Device
  if (glfwCreateWindowSurface(m_BackendData->Instance,
                              (GLFWwindow *)windowHandle, nullptr,
                              &m_BackendData->Surface) != VK_SUCCESS) {
    DO_CORE_ERROR("Failed to create Window Surface!");
    return false;
  }

  uint32_t gpuCount = 0;
  vkEnumeratePhysicalDevices(m_BackendData->Instance, &gpuCount, nullptr);
  std::vector<VkPhysicalDevice> gpus(gpuCount);
  vkEnumeratePhysicalDevices(m_BackendData->Instance, &gpuCount, gpus.data());
  m_BackendData->PhysicalDevice = gpus[0];

  // 3. Device & Extensions
  // Grafik kuyruğunu destekleyen queue family'yi bul
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_BackendData->PhysicalDevice, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_BackendData->PhysicalDevice, &queueFamilyCount, queueFamilies.data());

  uint32_t graphicsQueueFamily = 0;
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsQueueFamily = i;
      break;
    }
  }

  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  queueInfo.queueFamilyIndex = graphicsQueueFamily;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = &queuePriority;  std::vector<const char *> devExts = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
      VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
      VK_KHR_MAINTENANCE2_EXTENSION_NAME,
      VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
      VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
  };

  // Vulkan 1.2 Features
  VkPhysicalDeviceVulkan12Features vk12Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  vk12Features.bufferDeviceAddress = VK_TRUE;
  vk12Features.timelineSemaphore = VK_TRUE;
  vk12Features.runtimeDescriptorArray = VK_TRUE;
  vk12Features.descriptorBindingPartiallyBound = VK_TRUE;

  // Vulkan 1.3 Features
  VkPhysicalDeviceVulkan13Features vk13Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  vk13Features.pNext = &vk12Features; 
  vk13Features.dynamicRendering = VK_TRUE;
  // vk13Features.synchronization2 = VK_TRUE; // Disabled for stabilization test

  // Basic Features wrapping
  VkPhysicalDeviceFeatures2 deviceFeatures2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  deviceFeatures2.pNext = &vk13Features; 
  deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
  deviceFeatures2.features.fillModeNonSolid = VK_TRUE;
  deviceFeatures2.features.shaderInt64 = VK_TRUE;

  VkDeviceCreateInfo devInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  devInfo.pNext = &deviceFeatures2;
  devInfo.queueCreateInfoCount = 1;
  devInfo.pQueueCreateInfos = &queueInfo;
  devInfo.enabledExtensionCount = static_cast<uint32_t>(devExts.size());
  devInfo.ppEnabledExtensionNames = devExts.data();

  if (vkCreateDevice(m_BackendData->PhysicalDevice, &devInfo, nullptr, &m_BackendData->Device) != VK_SUCCESS) {
    DO_CORE_ERROR("Failed to create Vulkan Logical Device!");
    return false;
  }
  vkGetDeviceQueue(m_BackendData->Device, graphicsQueueFamily, 0, &m_BackendData->GraphicsQueue);
  m_BackendData->GraphicsQueueFamily = graphicsQueueFamily;

  // =================================================================================
  // Load device-level function pointers
  VULKAN_HPP_DEFAULT_DISPATCHER.init(vk::Device(m_BackendData->Device));
  // =================================================================================

  DO_CORE_INFO("RHI: Initializing NVRHI Device...");
  
  nvrhi::vulkan::DeviceDesc nvrhiDesc;
  nvrhiDesc.instance = m_BackendData->Instance;
  nvrhiDesc.physicalDevice = m_BackendData->PhysicalDevice;
  nvrhiDesc.device = m_BackendData->Device;
  nvrhiDesc.graphicsQueue = m_BackendData->GraphicsQueue;
  nvrhiDesc.graphicsQueueIndex = m_BackendData->GraphicsQueueFamily;
  nvrhiDesc.deviceExtensions = devExts.data();
  nvrhiDesc.numDeviceExtensions = devExts.size();
  nvrhiDesc.bufferDeviceAddressSupported = false; // Disabled for stabilization test

  // Inform NVRHI about enabled extensions so it can load the correct functions
  nvrhiDesc.instanceExtensions = exts.data();
  nvrhiDesc.numInstanceExtensions = exts.size();
  nvrhiDesc.deviceExtensions = devExts.data();
  nvrhiDesc.numDeviceExtensions = devExts.size();

  DO_CORE_INFO("RHI: Calling nvrhi::vulkan::createDevice...");
  m_BackendData->m_Device = nvrhi::vulkan::createDevice(nvrhiDesc);
  
  if (m_BackendData->m_Device) {
    DO_CORE_INFO("RHI: NVRHI Device created successfully.");
  } else {
    DO_CORE_ERROR("RHI: FAILED to create NVRHI Device!");
    return false;
  }

      // 5. Swapchain
      int width,
  height;
  glfwGetFramebufferSize((GLFWwindow *)windowHandle, &width, &height);
  if (width == 0 || height == 0) {
    width = 1280;
    height = 720;
  }

  VkSwapchainCreateInfoKHR swInfo = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swInfo.surface = m_BackendData->Surface;
  swInfo.minImageCount = 3;
  swInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  swInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  swInfo.imageExtent = {(uint32_t)width, (uint32_t)height};
  swInfo.imageArrayLayers = 1;
  swInfo.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  swInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  swInfo.clipped = VK_TRUE;

  vkCreateSwapchainKHR(m_BackendData->Device, &swInfo, nullptr,
                       &m_BackendData->SwapChain);

  // 6. Wrap Swapchain Images for NVRHI
  uint32_t imgCount;
  vkGetSwapchainImagesKHR(m_BackendData->Device, m_BackendData->SwapChain,
                          &imgCount, nullptr);
  std::vector<VkImage> images(imgCount);
  vkGetSwapchainImagesKHR(m_BackendData->Device, m_BackendData->SwapChain,
                          &imgCount, images.data());

  auto *vkDevice =
      static_cast<nvrhi::vulkan::IDevice *>(m_BackendData->m_Device.Get());
  for (auto img : images) {
    nvrhi::TextureDesc td;
    td.width = width;
    td.height = height;
    td.format = nvrhi::Format::BGRA8_UNORM;
    td.isRenderTarget = true;
    td.initialState = nvrhi::ResourceStates::Present;
    td.keepInitialState = true;

    auto tex = vkDevice->createHandleForNativeTexture(
        nvrhi::ObjectTypes::VK_Image, nvrhi::Object(img), td);
    m_BackendData->SwapChainTextures.push_back(tex);

    nvrhi::FramebufferDesc fbd;
    fbd.addColorAttachment(tex);
    m_BackendData->SwapChainFramebuffers.push_back(
        m_BackendData->m_Device->createFramebuffer(fbd));
  }

  VkSemaphoreCreateInfo semInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  vkCreateSemaphore(m_BackendData->Device, &semInfo, nullptr,
                    &m_BackendData->ImageAvailableSemaphore);
  vkCreateSemaphore(m_BackendData->Device, &semInfo, nullptr,
                    &m_BackendData->RenderFinishedSemaphore);

  DO_LOG("RHI Vulkan Backend Initialized.");
  return true;
}

void RHIDevice::WaitForIdle() {
  if (m_BackendData->m_Device)
    m_BackendData->m_Device->waitForIdle();
}

RHIDevice::VulkanHandles RHIDevice::GetVulkanHandles() const {
  VulkanHandles h;
  h.Instance       = m_BackendData->Instance;
  h.PhysicalDevice = m_BackendData->PhysicalDevice;
  h.Device         = m_BackendData->Device;
  h.GraphicsQueue  = m_BackendData->GraphicsQueue;
  h.QueueFamily    = m_BackendData->GraphicsQueueFamily;
  h.MinImageCount  = 2;
  h.ImageCount     = (uint32_t)m_BackendData->SwapChainFramebuffers.size();
  return h;
}

nvrhi::IDevice *RHIDevice::GetDevice() const {
  return m_BackendData->m_Device.Get();
}

nvrhi::IFramebuffer *RHIDevice::GetCurrentFramebuffer() const {
  return m_BackendData
      ->SwapChainFramebuffers[m_BackendData->CurrentSwapChainIndex];
}

nvrhi::CommandListHandle RHIDevice::CreateCommandList() {
  return m_BackendData->m_Device->createCommandList();
}

nvrhi::CommandListHandle RHIDevice::GetCurrentCommandList() const {
  return m_BackendData->m_CurrentCommandList;
}

void RHIDevice::ExecuteCommandList(nvrhi::CommandListHandle cmdList) {
  m_BackendData->m_Device->executeCommandList(cmdList);
}

void RHIDevice::Shutdown() {
  WaitForIdle();
  if (m_BackendData->m_Device) {
    m_BackendData->SwapChainFramebuffers.clear();
    m_BackendData->SwapChainTextures.clear();
    m_BackendData->m_Device = nullptr;
  }
  if (m_BackendData->ImageAvailableSemaphore)
    vkDestroySemaphore(m_BackendData->Device,
                       m_BackendData->ImageAvailableSemaphore, nullptr);
  if (m_BackendData->RenderFinishedSemaphore)
    vkDestroySemaphore(m_BackendData->Device,
                       m_BackendData->RenderFinishedSemaphore, nullptr);
  if (m_BackendData->SwapChain)
    vkDestroySwapchainKHR(m_BackendData->Device, m_BackendData->SwapChain,
                          nullptr);
  if (m_BackendData->Device)
    vkDestroyDevice(m_BackendData->Device, nullptr);
  if (m_BackendData->Surface)
    vkDestroySurfaceKHR(m_BackendData->Instance, m_BackendData->Surface,
                        nullptr);
  if (m_BackendData->Instance)
    vkDestroyInstance(m_BackendData->Instance, nullptr);
}

void RHIDevice::BeginFrame() {
  vkAcquireNextImageKHR(m_BackendData->Device, m_BackendData->SwapChain,
                        UINT64_MAX, m_BackendData->ImageAvailableSemaphore,
                        VK_NULL_HANDLE, &m_BackendData->CurrentSwapChainIndex);
  static_cast<nvrhi::vulkan::IDevice *>(m_BackendData->m_Device.Get())
      ->queueWaitForSemaphore(nvrhi::CommandQueue::Graphics,
                              m_BackendData->ImageAvailableSemaphore, 0);

  m_BackendData->m_CurrentCommandList =
      m_BackendData->m_Device->createCommandList(
          nvrhi::CommandListParameters().setEnableImmediateExecution(false));
  m_BackendData->m_CurrentCommandList->open();
}

void RHIDevice::EndFrame() {
  if (m_BackendData->m_CurrentCommandList) {
    auto tex =
        m_BackendData->SwapChainTextures[m_BackendData->CurrentSwapChainIndex];
    m_BackendData->m_CurrentCommandList->setTextureState(
        tex, nvrhi::AllSubresources, nvrhi::ResourceStates::Present);
    m_BackendData->m_CurrentCommandList->close();
    m_BackendData->m_Device->executeCommandList(
        m_BackendData->m_CurrentCommandList);

    static_cast<nvrhi::vulkan::IDevice *>(m_BackendData->m_Device.Get())
        ->queueSignalSemaphore(nvrhi::CommandQueue::Graphics,
                               m_BackendData->RenderFinishedSemaphore, 0);
    m_BackendData->m_CurrentCommandList = nullptr;
  }

  VkPresentInfoKHR prInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  prInfo.waitSemaphoreCount = 1;
  prInfo.pWaitSemaphores = &m_BackendData->RenderFinishedSemaphore;
  prInfo.swapchainCount = 1;
  prInfo.pSwapchains = &m_BackendData->SwapChain;
  prInfo.pImageIndices = &m_BackendData->CurrentSwapChainIndex;
  vkQueuePresentKHR(m_BackendData->GraphicsQueue, &prInfo);

  m_BackendData->m_Device->runGarbageCollection();
}

void RHIDevice::Clear(float r, float g, float b, float a) {
  if (!m_BackendData->m_CurrentCommandList)
    return;
  auto tex =
      m_BackendData->SwapChainTextures[m_BackendData->CurrentSwapChainIndex];
  m_BackendData->m_CurrentCommandList->setTextureState(
      tex, nvrhi::AllSubresources, nvrhi::ResourceStates::RenderTarget);
  m_BackendData->m_CurrentCommandList->clearTextureFloat(
      tex, nvrhi::AllSubresources, nvrhi::Color(r, g, b, a));
}

// --- Resource Creation Implementations ---

BufferHandle RHIDevice::CreateBuffer(const BufferDesc &desc) {
  nvrhi::BufferDesc nvrhiDesc;
  nvrhiDesc.byteSize = desc.byteSize;
  nvrhiDesc.isVertexBuffer = desc.isVertexBuffer;
  nvrhiDesc.isIndexBuffer = desc.isIndexBuffer;
  nvrhiDesc.isConstantBuffer = desc.isConstantBuffer;
  nvrhiDesc.debugName = desc.debugName;
  return std::make_shared<ResourceWrapper<nvrhi::IBuffer>>(
      m_BackendData->m_Device->createBuffer(nvrhiDesc));
}

TextureHandle RHIDevice::CreateTexture(const TextureDesc &desc) {
  nvrhi::TextureDesc nvrhiDesc;
  nvrhiDesc.width = desc.width;
  nvrhiDesc.height = desc.height;
  nvrhiDesc.depth = desc.depth;
  nvrhiDesc.mipLevels = desc.mipLevels;
  nvrhiDesc.debugName = desc.debugName;
  return std::make_shared<ResourceWrapper<nvrhi::ITexture>>(
      m_BackendData->m_Device->createTexture(nvrhiDesc));
}

SamplerHandle RHIDevice::CreateSampler() {
  nvrhi::SamplerDesc desc;
  return std::make_shared<ResourceWrapper<nvrhi::ISampler>>(
      m_BackendData->m_Device->createSampler(desc));
}

ShaderHandle RHIDevice::CreateShader(const char *entryPoint, const char *stage,
                                     const void *binary, size_t size) {
  nvrhi::ShaderType type = nvrhi::ShaderType::None;
  if (std::string(stage) == "vertex")
    type = nvrhi::ShaderType::Vertex;
  else if (std::string(stage) == "pixel")
    type = nvrhi::ShaderType::Pixel;

  nvrhi::ShaderDesc desc(type);
  desc.entryName = entryPoint;
  return std::make_shared<ResourceWrapper<nvrhi::IShader>>(
      m_BackendData->m_Device->createShader(desc, binary, size));
}

InputLayoutHandle
RHIDevice::CreateInputLayout(const VertexAttribute *attributes,
                             uint32_t count) {
  std::vector<nvrhi::VertexAttributeDesc> vadescs;
  for (uint32_t i = 0; i < count; ++i) {
    nvrhi::VertexAttributeDesc d;
    d.name = attributes[i].name;
    d.offset = attributes[i].offset;
    d.elementStride = attributes[i].stride;
    vadescs.push_back(d);
  }
  return std::make_shared<ResourceWrapper<nvrhi::IInputLayout>>(
      m_BackendData->m_Device->createInputLayout(vadescs.data(), count,
                                                 nullptr));
}

BindingLayoutHandle
RHIDevice::CreateBindingLayout(const BindingLayoutItem *items, uint32_t count) {
  nvrhi::BindingLayoutDesc desc;
  for (uint32_t i = 0; i < count; ++i) {
    nvrhi::BindingLayoutItem item;
    item.slot = items[i].slot;
    if (items[i].type == BindingType::ConstantBuffer)
      item.type = nvrhi::ResourceType::ConstantBuffer;
    else if (items[i].type == BindingType::Texture)
      item.type = nvrhi::ResourceType::Texture_SRV;
    else if (items[i].type == BindingType::Sampler)
      item.type = nvrhi::ResourceType::Sampler;
    desc.bindings.push_back(item);
  }
  return std::make_shared<ResourceWrapper<nvrhi::IBindingLayout>>(
      m_BackendData->m_Device->createBindingLayout(desc));
}

BindingSetHandle RHIDevice::CreateBindingSet(
    BindingLayoutHandle layout,
    const std::vector<std::pair<uint32_t, std::shared_ptr<IResource>>>
        &resources) {
  nvrhi::BindingSetDesc desc;
  auto nativeLayout =
      static_cast<ResourceWrapper<nvrhi::IBindingLayout> *>(layout.get())
          ->handle;
  for (const auto &res : resources) {
    nvrhi::BindingSetItem item;
    item.slot = res.first;
    auto raw = res.second.get();
    if (auto b = dynamic_cast<ResourceWrapper<nvrhi::IBuffer> *>(raw))
      item.resourceHandle = b->handle;
    else if (auto t = dynamic_cast<ResourceWrapper<nvrhi::ITexture> *>(raw))
      item.resourceHandle = t->handle;
    else if (auto s = dynamic_cast<ResourceWrapper<nvrhi::ISampler> *>(raw))
      item.resourceHandle = s->handle;
    desc.bindings.push_back(item);
  }
  return std::make_shared<ResourceWrapper<nvrhi::IBindingSet>>(
      m_BackendData->m_Device->createBindingSet(desc, nativeLayout));
}

PipelineHandle RHIDevice::CreateGraphicsPipeline(const PipelineDesc &desc) {
  nvrhi::GraphicsPipelineDesc nvrhiDesc;
  nvrhiDesc.VS =
      static_cast<ResourceWrapper<nvrhi::IShader> *>(desc.vertexShader.get())
          ->handle;
  nvrhiDesc.PS =
      static_cast<ResourceWrapper<nvrhi::IShader> *>(desc.pixelShader.get())
          ->handle;
  nvrhiDesc.inputLayout = static_cast<ResourceWrapper<nvrhi::IInputLayout> *>(
                              desc.inputLayout.get())
                              ->handle;
  nvrhiDesc.bindingLayouts = {
      static_cast<ResourceWrapper<nvrhi::IBindingLayout> *>(
          desc.bindingLayout.get())
          ->handle};

  nvrhiDesc.renderState.rasterState.fillMode = nvrhi::RasterFillMode::Solid;
  nvrhiDesc.renderState.rasterState.cullMode = nvrhi::RasterCullMode::None;

  return std::make_shared<ResourceWrapper<nvrhi::IGraphicsPipeline>>(
      m_BackendData->m_Device->createGraphicsPipeline(
          nvrhiDesc, m_BackendData->SwapChainFramebuffers[0]));
}

void RHIDevice::BindPipeline(PipelineHandle pipeline, BufferHandle vb) {
  if (!m_BackendData->m_CurrentCommandList)
    return;
  auto nativePipeline =
      static_cast<ResourceWrapper<nvrhi::IGraphicsPipeline> *>(pipeline.get())
          ->handle;
  m_BackendData->m_CurrentGraphicsState.pipeline = nativePipeline;
  m_BackendData->m_CurrentGraphicsState.framebuffer =
      m_BackendData
          ->SwapChainFramebuffers[m_BackendData->CurrentSwapChainIndex];

  if (vb) {
    nvrhi::VertexBufferBinding vbb;
    vbb.buffer =
        static_cast<ResourceWrapper<nvrhi::IBuffer> *>(vb.get())->handle;
    m_BackendData->m_CurrentGraphicsState.vertexBuffers = {vbb};
  }
}

void RHIDevice::BindBindingSet(uint32_t slot, BindingSetHandle set) {
  if (!m_BackendData->m_CurrentCommandList)
    return;
  auto nativeSet =
      static_cast<ResourceWrapper<nvrhi::IBindingSet> *>(set.get())->handle;

  if (m_BackendData->m_CurrentGraphicsState.bindings.size() <= slot) {
    m_BackendData->m_CurrentGraphicsState.bindings.resize(slot + 1);
  }
  m_BackendData->m_CurrentGraphicsState.bindings[slot] = nativeSet;
}

void RHIDevice::WriteBuffer(BufferHandle buffer, const void *data,
                            size_t size) {
  if (!m_BackendData->m_CurrentCommandList)
    return;
  auto native =
      static_cast<ResourceWrapper<nvrhi::IBuffer> *>(buffer.get())->handle;
  m_BackendData->m_CurrentCommandList->writeBuffer(native, data, size);
}

void RHIDevice::WriteTexture(TextureHandle texture, const void *data,
                             size_t size) {
  if (!m_BackendData->m_CurrentCommandList)
    return;
  auto native =
      static_cast<ResourceWrapper<nvrhi::ITexture> *>(texture.get())->handle;
  auto& desc = native->getDesc();
  
  // ImGui font atlas için genellikle RGBA8 (4 bytes)
  uint32_t bytesPerPixel = 4; 
  uint32_t rowPitch = desc.width * bytesPerPixel;
  
  m_BackendData->m_CurrentCommandList->writeTexture(native, 0, 0, data, rowPitch, size);
}

void RHIDevice::Draw(uint32_t vertexCount, uint32_t instanceCount) {
  if (!m_BackendData->m_CurrentCommandList)
    return;
  m_BackendData->m_CurrentCommandList->setGraphicsState(
      m_BackendData->m_CurrentGraphicsState);
  nvrhi::DrawArguments args;
  args.vertexCount = vertexCount;
  args.instanceCount = instanceCount;
  m_BackendData->m_CurrentCommandList->draw(args);
}

} // namespace DoEngine