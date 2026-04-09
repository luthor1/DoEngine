#include "Editor.h"
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>

// DİKKAT: <backends/imgui_impl_vulkan.h> ve <vulkan/vulkan.h> TAMAMEN SİLİNDİ!
// Bunun yerine NVRHI için yazılmış (veya Donut framework'ten alınmış) ImGui
// backend'i ekleniyor.
#include "ImGui_ImplNVRHI.h"

#include "../Core/Base.h"
#include "../Engine/Application.h"
#include "../Project/ProjectManager.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace DoEngine {

Editor::Editor(RHIDevice *rhi, GLFWwindow *window)
    : m_RHI(rhi), m_Window(window) {
  InitImGui();
  DO_CORE_INFO("Editor system initialized with Dear ImGui (RHI Backend).");
}

Editor::~Editor() {
  // Vulkan'ın vkDeviceWaitIdle fonksiyonu yerine RHI'nin kendi soyutlanmış
  // metodunu çağırıyoruz. Bu sayede DX12 modunda çalışırken DX12'nin bekleme
  // mantığı çalışır.
  m_RHI->WaitForIdle();

  ImGui_ImplNVRHI_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  DO_CORE_INFO("Editor system shut down.");
}

void Editor::InitImGui() {
  DO_CORE_INFO("InitImGui: Starting...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  DO_CORE_INFO("InitImGui: Initializing GLFW Backend...");
  ImGui_ImplGlfw_InitForOther(m_Window, true);

  DO_CORE_INFO("InitImGui: Initializing NVRHI Backend...");
  if (!ImGui_ImplNVRHI_Init(m_RHI->GetDevice())) {
    DO_CORE_ERROR("InitImGui: ImGui_ImplNVRHI_Init failed!");
  }
  DO_CORE_INFO("InitImGui: Done.");
}

void Editor::BeginFrame() {
  ImGui_ImplNVRHI_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Editor::OnUpdate(Scene *scene) {
  // ... (Bu kısımlar tamamen oyun mantığı olduğu için aynı kalır)
  ImGui::Begin("Scene Hierarchy");
  auto view = scene->View<TagComponent>();
  for (auto entity : view) {
    auto &tag = view.get<TagComponent>(entity);
    if (ImGui::Selectable(tag.Tag.c_str())) {
      // Select entity logic
    }
  }
  ImGui::End();

  ImGui::Begin("Entity Inspector");
  ImGui::Text("Select an entity to view details.");
  ImGui::End();

  ImGui::ShowDemoWindow();
}

void Editor::OnHubUpdate(Application *app) {
  // ... (Hub arayüz kodları tamamen aynı kalır)
}

void Editor::EndFrame() {
  ImGui::Render();

  // RHI tarafından o an aktif olan komut listesini alıyoruz.
  // Bu sayede sahnede çizilen diğer objelerle aynı komut kuyruğuna (render pass içine)
  // ImGui verilerini güvenle ekleyebiliriz.
  nvrhi::CommandListHandle cmdList = m_RHI->GetCurrentCommandList();

  if (cmdList) {
    auto fb = m_RHI->GetCurrentFramebuffer();
    if (fb) {
      ImGui_ImplNVRHI_RenderDrawData(ImGui::GetDrawData(), cmdList, fb);
    }
  }
}

} // namespace DoEngine