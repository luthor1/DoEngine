#pragma once

#include <nvrhi/nvrhi.h>
#include <imgui.h>

namespace DoEngine {

    bool ImGui_ImplNVRHI_Init(nvrhi::IDevice* device);
    void ImGui_ImplNVRHI_Shutdown();
    void ImGui_ImplNVRHI_NewFrame();
    void ImGui_ImplNVRHI_RenderDrawData(ImDrawData* draw_data, nvrhi::ICommandList* cmd, nvrhi::IFramebuffer* fb = nullptr);

    // Creates the font texture. Called by Init, but can be used to re-create if needed.
    bool ImGui_ImplNVRHI_CreateDeviceObjects();
    void ImGui_ImplNVRHI_InvalidateDeviceObjects();

} // namespace DoEngine
