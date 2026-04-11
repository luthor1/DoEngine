#pragma once

#include "../ECS/Scene.h"
#include "../Graphics/RHI.h"
#include <string>
#include <vector>

struct GLFWwindow;

namespace DoEngine {

class Application;

using EntityID = entt::entity;
constexpr EntityID NullEntity = entt::null;

class Editor {
public:
  Editor(RHIDevice *rhi, GLFWwindow *window);
  ~Editor();

  void BeginFrame();
  void EndFrame();

  void OnHubUpdate(Application *app);
  void OnUpdate(Scene *scene);

  float GetViewportWidth()  const { return m_ViewportSize[0]; }
  float GetViewportHeight() const { return m_ViewportSize[1]; }

  static void AddLog(const std::string& msg);

private:
  void InitImGui();
  void ApplyDarkTheme();

  void DrawHub(Application* app);
  void DrawMenuBar(Scene* scene);
  void DrawOutliner(Scene* scene);
  void DrawInspector(Scene* scene);
  void DrawViewport();
  void DrawContentBrowser();
  void DrawLogPanel();
  void CreateEntityPopup(Scene* scene);

private:
  RHIDevice  *m_RHI    = nullptr;
  GLFWwindow *m_Window = nullptr;

  // Hub state
  char m_NewProjectName[256] = {};
  char m_NewProjectPath[512] = {};

  // Editor state
  EntityID m_SelectedEntity = NullEntity;
  char     m_ContentBrowserPath[512] = {};
  bool     m_ShowCreateEntityPopup = false;
  bool     m_LogAutoScroll = true;

  float m_ViewportSize[2] = {0.0f, 0.0f};

  static std::vector<std::string> s_LogMessages;
};

} // namespace DoEngine