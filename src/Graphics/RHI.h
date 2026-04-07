#pragma once

#include "../Core/Base.h"
#include <nvrhi/nvrhi.h>
#include <memory>

namespace DoEngine {

    class RHIDevice {
    public:
        RHIDevice();
        ~RHIDevice();

        bool Initialize(void* windowHandle);
        void Shutdown();

        // Rendering commands
        void BeginFrame();
        void EndFrame();
        void Clear(float r, float g, float b, float a);

        nvrhi::IDevice* GetNativeDevice() { return m_Device.Get(); }

    private:
        nvrhi::DeviceHandle m_Device;
        // Backend specific data (Vulkan/DX12) would go here
    };

}
