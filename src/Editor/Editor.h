#pragma once

#include "../ECS/Scene.h"
#include "../Graphics/RHI.h"

// DİKKAT: <vulkan/vulkan.h> sızıntıyı önlemek için TAMAMEN SİLİNDİ.

struct GLFWwindow;

namespace DoEngine {

class Application; // OnHubUpdate için Forward Declaration

class Editor {
public:
  Editor(RHIDevice *rhi, GLFWwindow *window);
  ~Editor();

  void BeginFrame();
  void EndFrame();

  void OnUpdate(Scene *scene);
  void OnHubUpdate(Application *app);

private:
  void InitImGui();

  // DİKKAT: CreateDescriptorPool() fonksiyonu silindi.
  // Kaynak yönetimi artık ImGui_ImplNVRHI backend'i tarafından yapılıyor.

private:
  RHIDevice *m_RHI;
  GLFWwindow *m_Window;

  // DİKKAT: VkDescriptorPool m_DescriptorPool değişkeni silindi.
};

} // namespace DoEngine