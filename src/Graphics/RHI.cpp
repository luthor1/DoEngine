#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <iostream>
#include <vector>
#include <string>

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <nvrhi/vulkan.h>
#include <GLFW/glfw3.h>

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "RHI.h"
#include "../Core/Base.h"

namespace DoEngine {

    // Wrapper to hold NVRHI resources inside our abstract handles
    template<typename T>
    struct ResourceWrapper : public IResource {
        nvrhi::RefCountPtr<T> handle;
        ResourceWrapper(T* p) : handle(p) {}
    };

    struct RHIDevice::BackendData {
        nvrhi::DeviceHandle m_Device;
        VkInstance Instance = VK_NULL_HANDLE;
        VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
        VkDevice Device = VK_NULL_HANDLE;
        VkQueue GraphicsQueue = VK_NULL_HANDLE;
        uint32_t GraphicsQueueIndex = 0;
        
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

    RHIDevice::~RHIDevice() {
        Shutdown();
    }

    bool RHIDevice::Initialize(GraphicsAPI api, void* windowHandle) {
        m_CurrentAPI = api;
        if (api == GraphicsAPI::Vulkan) {
            return InitVulkan(windowHandle);
        }
        return false;
    }

    bool RHIDevice::InitVulkan(void* windowHandle) {
        DO_LOG("Initializing RHI Vulkan Backend...");
        
        // 1. Instance
        VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.apiVersion = VK_API_VERSION_1_2;

        uint32_t glfwExtCount = 0;
        const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
        std::vector<const char*> exts(glfwExts, glfwExts + glfwExtCount);

        VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instInfo.pApplicationInfo = &appInfo;
        instInfo.enabledExtensionCount = static_cast<uint32_t>(exts.size());
        instInfo.ppEnabledExtensionNames = exts.data();

        if (vkCreateInstance(&instInfo, nullptr, &m_BackendData->Instance) != VK_SUCCESS) return false;
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vk::Instance(m_BackendData->Instance), vkGetInstanceProcAddr);

        // 2. Surface & Physical Device
        glfwCreateWindowSurface(m_BackendData->Instance, (GLFWwindow*)windowHandle, nullptr, &m_BackendData->Surface);
        
        uint32_t gpuCount = 1;
        vkEnumeratePhysicalDevices(m_BackendData->Instance, &gpuCount, &m_BackendData->PhysicalDevice);
        
        // 3. Device & Queue
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        const char* deviceExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        devInfo.queueCreateInfoCount = 1;
        devInfo.pQueueCreateInfos = &queueInfo;
        devInfo.enabledExtensionCount = 1;
        devInfo.ppEnabledExtensionNames = deviceExts;

        vkCreateDevice(m_BackendData->PhysicalDevice, &devInfo, nullptr, &m_BackendData->Device);
        vkGetDeviceQueue(m_BackendData->Device, 0, 0, &m_BackendData->GraphicsQueue);

        // 4. NVRHI
        nvrhi::vulkan::DeviceDesc nvrhiDesc;
        nvrhiDesc.instance = m_BackendData->Instance;
        nvrhiDesc.physicalDevice = m_BackendData->PhysicalDevice;
        nvrhiDesc.device = m_BackendData->Device;
        nvrhiDesc.graphicsQueue = m_BackendData->GraphicsQueue;
        nvrhiDesc.graphicsQueueIndex = 0;
        
        m_BackendData->m_Device = nvrhi::vulkan::createDevice(nvrhiDesc);

        // 5. Swapchain
        VkSwapchainCreateInfoKHR swInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        swInfo.surface = m_BackendData->Surface;
        swInfo.minImageCount = 3;
        swInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        swInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swInfo.imageExtent = { 1280, 720 };
        swInfo.imageArrayLayers = 1;
        swInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        swInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swInfo.clipped = VK_TRUE;

        vkCreateSwapchainKHR(m_BackendData->Device, &swInfo, nullptr, &m_BackendData->SwapChain);

        // 6. Sync Objects & Swapchain Textures
        uint32_t imgCount;
        vkGetSwapchainImagesKHR(m_BackendData->Device, m_BackendData->SwapChain, &imgCount, nullptr);
        std::vector<VkImage> images(imgCount);
        vkGetSwapchainImagesKHR(m_BackendData->Device, m_BackendData->SwapChain, &imgCount, images.data());

        auto* vkDevice = static_cast<nvrhi::vulkan::IDevice*>(m_BackendData->m_Device.Get());
        for (auto img : images) {
            nvrhi::TextureDesc td;
            td.width = 1280; td.height = 720; td.format = nvrhi::Format::BGRA8_UNORM;
            td.isRenderTarget = true; td.debugName = "SwapChainImg";
            td.initialState = nvrhi::ResourceStates::Present;
            td.keepInitialState = true;

            auto tex = vkDevice->createHandleForNativeTexture(nvrhi::ObjectTypes::VK_Image, nvrhi::Object(img), td);
            m_BackendData->SwapChainTextures.push_back(tex);

            nvrhi::FramebufferDesc fbd;
            fbd.addColorAttachment(tex);
            m_BackendData->SwapChainFramebuffers.push_back(m_BackendData->m_Device->createFramebuffer(fbd));
        }

        VkSemaphoreCreateInfo semInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore(m_BackendData->Device, &semInfo, nullptr, &m_BackendData->ImageAvailableSemaphore);
        vkCreateSemaphore(m_BackendData->Device, &semInfo, nullptr, &m_BackendData->RenderFinishedSemaphore);

        DO_LOG("RHI Vulkan Backend Initialized.");
        return true;
    }

    bool RHIDevice::InitD3D12(void*) { return false; }

    void RHIDevice::Shutdown() {
        if (m_BackendData->m_Device) {
            m_BackendData->m_Device->waitForIdle();
            m_BackendData->SwapChainFramebuffers.clear();
            m_BackendData->SwapChainTextures.clear();
            m_BackendData->m_Device = nullptr;
        }
        if (m_BackendData->ImageAvailableSemaphore) vkDestroySemaphore(m_BackendData->Device, m_BackendData->ImageAvailableSemaphore, nullptr);
        if (m_BackendData->RenderFinishedSemaphore) vkDestroySemaphore(m_BackendData->Device, m_BackendData->RenderFinishedSemaphore, nullptr);
        if (m_BackendData->SwapChain) vkDestroySwapchainKHR(m_BackendData->Device, m_BackendData->SwapChain, nullptr);
        if (m_BackendData->Device) vkDestroyDevice(m_BackendData->Device, nullptr);
        if (m_BackendData->Surface) vkDestroySurfaceKHR(m_BackendData->Instance, m_BackendData->Surface, nullptr);
        if (m_BackendData->Instance) vkDestroyInstance(m_BackendData->Instance, nullptr);
    }

    void RHIDevice::BeginFrame() {
        vkAcquireNextImageKHR(m_BackendData->Device, m_BackendData->SwapChain, UINT64_MAX, m_BackendData->ImageAvailableSemaphore, VK_NULL_HANDLE, &m_BackendData->CurrentSwapChainIndex);
        auto* vkDevice = static_cast<nvrhi::vulkan::IDevice*>(m_BackendData->m_Device.Get());
        vkDevice->queueWaitForSemaphore(nvrhi::CommandQueue::Graphics, m_BackendData->ImageAvailableSemaphore, 0);

        m_BackendData->m_CurrentCommandList = m_BackendData->m_Device->createCommandList();
        m_BackendData->m_CurrentCommandList->open();
    }

    void RHIDevice::EndFrame() {
        if (m_BackendData->m_CurrentCommandList) {
            auto tex = m_BackendData->SwapChainTextures[m_BackendData->CurrentSwapChainIndex];
            m_BackendData->m_CurrentCommandList->setTextureState(tex, nvrhi::AllSubresources, nvrhi::ResourceStates::Present);
            m_BackendData->m_CurrentCommandList->close();
            m_BackendData->m_Device->executeCommandList(m_BackendData->m_CurrentCommandList);
            
            auto* vkDevice = static_cast<nvrhi::vulkan::IDevice*>(m_BackendData->m_Device.Get());
            vkDevice->queueSignalSemaphore(nvrhi::CommandQueue::Graphics, m_BackendData->RenderFinishedSemaphore, 0);
            m_BackendData->m_CurrentCommandList = nullptr;
        }

        VkPresentInfoKHR prInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        prInfo.waitSemaphoreCount = 1;
        prInfo.pWaitSemaphores = &m_BackendData->RenderFinishedSemaphore;
        prInfo.swapchainCount = 1;
        prInfo.pSwapchains = &m_BackendData->SwapChain;
        prInfo.pImageIndices = &m_BackendData->CurrentSwapChainIndex;
        vkQueuePresentKHR(m_BackendData->GraphicsQueue, &prInfo);
        
        m_BackendData->m_Device->runGarbageCollection();
    }

    void RHIDevice::Clear(float r, float g, float b, float a) {
        if (!m_BackendData->m_CurrentCommandList) return;
        auto tex = m_BackendData->SwapChainTextures[m_BackendData->CurrentSwapChainIndex];
        m_BackendData->m_CurrentCommandList->setTextureState(tex, nvrhi::AllSubresources, nvrhi::ResourceStates::RenderTarget);
        m_BackendData->m_CurrentCommandList->clearTextureFloat(tex, nvrhi::AllSubresources, nvrhi::Color(r, g, b, a));
    }

    BufferHandle RHIDevice::CreateBuffer(const BufferDesc& desc) {
        nvrhi::BufferDesc d;
        d.byteSize = desc.byteSize;
        d.isVertexBuffer = desc.isVertexBuffer;
        d.isIndexBuffer = desc.isIndexBuffer;
        d.isConstantBuffer = desc.isConstantBuffer;
        d.debugName = desc.debugName;
        return std::make_shared<ResourceWrapper<nvrhi::IBuffer>>(m_BackendData->m_Device->createBuffer(d).Get());
    }

    ShaderHandle RHIDevice::CreateShader(const char* entry, const char* stage, const void* bin, size_t sz) {
        nvrhi::ShaderDesc d;
        d.entryName = entry;
        d.shaderType = (std::string(stage) == "vs") ? nvrhi::ShaderType::Vertex : nvrhi::ShaderType::Pixel;
        return std::make_shared<ResourceWrapper<nvrhi::IShader>>(m_BackendData->m_Device->createShader(d, bin, sz).Get());
    }

    InputLayoutHandle RHIDevice::CreateInputLayout(const VertexAttribute* attrs, uint32_t count) {
        std::vector<nvrhi::VertexAttributeDesc> nvAttrs;
        for(uint32_t i=0; i<count; ++i) {
            nvAttrs.push_back(nvrhi::VertexAttributeDesc().setName(attrs[i].name).setFormat(nvrhi::Format::RGBA32_FLOAT).setOffset(attrs[i].offset).setElementStride(attrs[i].stride));
        }
        return std::make_shared<ResourceWrapper<nvrhi::IInputLayout>>(m_BackendData->m_Device->createInputLayout(nvAttrs.data(), count, nullptr).Get());
    }

    BindingLayoutHandle RHIDevice::CreateBindingLayout(uint32_t slot) {
        nvrhi::BindingLayoutDesc d;
        d.addItem(nvrhi::BindingLayoutItem::ConstantBuffer(slot));
        return std::make_shared<ResourceWrapper<nvrhi::IBindingLayout>>(m_BackendData->m_Device->createBindingLayout(d).Get());
    }

    BindingSetHandle RHIDevice::CreateBindingSet(BindingLayoutHandle layout, BufferHandle buffer) {
        nvrhi::BindingSetDesc d;
        auto* rawBuf = static_cast<ResourceWrapper<nvrhi::IBuffer>*>(buffer.get())->handle.Get();
        d.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, rawBuf));
        auto* rawLayout = static_cast<ResourceWrapper<nvrhi::IBindingLayout>*>(layout.get())->handle.Get();
        return std::make_shared<ResourceWrapper<nvrhi::IBindingSet>>(m_BackendData->m_Device->createBindingSet(d, rawLayout).Get());
    }

    PipelineHandle RHIDevice::CreateGraphicsPipeline(const PipelineDesc& desc) {
        nvrhi::GraphicsPipelineDesc d;
        d.VS = static_cast<ResourceWrapper<nvrhi::IShader>*>(desc.vertexShader.get())->handle.Get();
        d.PS = static_cast<ResourceWrapper<nvrhi::IShader>*>(desc.pixelShader.get())->handle.Get();
        d.inputLayout = static_cast<ResourceWrapper<nvrhi::IInputLayout>*>(desc.inputLayout.get())->handle.Get();
        d.addBindingLayout(static_cast<ResourceWrapper<nvrhi::IBindingLayout>*>(desc.bindingLayout.get())->handle.Get());
        d.renderState.rasterState.cullMode = nvrhi::RasterCullMode::None;
        return std::make_shared<ResourceWrapper<nvrhi::IGraphicsPipeline>>(m_BackendData->m_Device->createGraphicsPipeline(d, nullptr).Get());
    }

    void RHIDevice::BindPipeline(PipelineHandle pipe, BufferHandle vb) {
        if (!m_BackendData->m_CurrentCommandList) return;
        auto* rawPipe = static_cast<ResourceWrapper<nvrhi::IGraphicsPipeline>*>(pipe.get())->handle.Get();
        m_BackendData->m_CurrentGraphicsState = nvrhi::GraphicsState();
        m_BackendData->m_CurrentGraphicsState.pipeline = rawPipe;
        m_BackendData->m_CurrentGraphicsState.framebuffer = m_BackendData->SwapChainFramebuffers[m_BackendData->CurrentSwapChainIndex];
        auto vp = m_BackendData->m_CurrentGraphicsState.framebuffer->getDesc().colorAttachments[0].texture->getDesc();
        m_BackendData->m_CurrentGraphicsState.viewport.addViewportAndScissorRect(nvrhi::Viewport(float(vp.width), float(vp.height)));
        if (vb) {
            auto* rawVB = static_cast<ResourceWrapper<nvrhi::IBuffer>*>(vb.get())->handle.Get();
            m_BackendData->m_CurrentGraphicsState.vertexBuffers.push_back({ rawVB, 0, 0 });
        }
    }

    void RHIDevice::BindBindingSet(uint32_t slot, BindingSetHandle set) {
        auto* rawSet = static_cast<ResourceWrapper<nvrhi::IBindingSet>*>(set.get())->handle.Get();
        if (slot >= m_BackendData->m_CurrentGraphicsState.bindings.size()) m_BackendData->m_CurrentGraphicsState.bindings.resize(slot + 1);
        m_BackendData->m_CurrentGraphicsState.bindings[slot] = rawSet;
    }

    void RHIDevice::WriteBuffer(BufferHandle buffer, const void* data, size_t size) {
        if (!m_BackendData->m_CurrentCommandList) return;
        auto* rawBuf = static_cast<ResourceWrapper<nvrhi::IBuffer>*>(buffer.get())->handle.Get();
        m_BackendData->m_CurrentCommandList->writeBuffer(rawBuf, data, size);
    }

    void RHIDevice::Draw(uint32_t count, uint32_t instances) {
        if (!m_BackendData->m_CurrentCommandList) return;
        m_BackendData->m_CurrentCommandList->setGraphicsState(m_BackendData->m_CurrentGraphicsState);
        nvrhi::DrawArguments args; args.vertexCount = count; args.instanceCount = instances;
        m_BackendData->m_CurrentCommandList->draw(args);
    }
}
