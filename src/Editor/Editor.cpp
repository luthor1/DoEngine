#include "Editor.h"

#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
#include "ImGui_ImplNVRHI.h"

#include "../Core/Base.h"
#include "../Engine/Application.h"
#include "../Project/ProjectManager.h"
#include "../ECS/Components.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <cstring>
#include <algorithm>

namespace DoEngine {

std::vector<std::string> Editor::s_LogMessages;

void Editor::AddLog(const std::string& msg) {
    if (s_LogMessages.size() > 500)
        s_LogMessages.erase(s_LogMessages.begin());
    s_LogMessages.push_back(msg);
}

// ─────────────────────────────────────────────────────────────────────────────
Editor::Editor(RHIDevice *rhi, GLFWwindow *window)
    : m_RHI(rhi), m_Window(window)
{
    InitImGui();

    const char* home = std::getenv("USERPROFILE");
    if (home) {
        snprintf(m_NewProjectPath, sizeof(m_NewProjectPath), "%s\\Documents", home);
        snprintf(m_ContentBrowserPath, sizeof(m_ContentBrowserPath), "%s\\Documents", home);
    }

    AddLog("[Engine] DoEngine baslatildi.");
    DO_CORE_INFO("Editor initialized (NVRHI backend).");
}

Editor::~Editor() {
    m_RHI->WaitForIdle();
    ImGui_ImplNVRHI_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// ─────────────────────────────────────────────────────────────────────────────
void Editor::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ApplyDarkTheme();
    // 'RendererHasTextures' set olduğu için io.Fonts->Build() çağrılmaz!
    // Backend (NVRHI) texture upload'ı lazy yapıyor.
    io.Fonts->AddFontDefault();

    ImGui_ImplGlfw_InitForOther(m_Window, true);

    if (!ImGui_ImplNVRHI_Init(m_RHI)) {
        DO_CORE_ERROR("ImGui_ImplNVRHI_Init failed!");
    }
}

void Editor::ApplyDarkTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding  = 5.0f;  s.ChildRounding  = 4.0f;
    s.FrameRounding   = 4.0f;  s.PopupRounding  = 4.0f;
    s.GrabRounding    = 4.0f;  s.TabRounding    = 4.0f;
    s.WindowPadding   = {10, 10}; s.FramePadding = {6, 4};
    s.ItemSpacing     = {8, 5};   s.ScrollbarSize = 11.0f;
    s.IndentSpacing   = 16.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]       = {0.09f, 0.09f, 0.13f, 1.00f};
    c[ImGuiCol_ChildBg]        = {0.10f, 0.10f, 0.15f, 1.00f};
    c[ImGuiCol_PopupBg]        = {0.10f, 0.10f, 0.15f, 0.98f};
    c[ImGuiCol_Border]         = {0.20f, 0.20f, 0.30f, 1.00f};
    c[ImGuiCol_FrameBg]        = {0.14f, 0.14f, 0.20f, 1.00f};
    c[ImGuiCol_FrameBgHovered] = {0.20f, 0.20f, 0.30f, 1.00f};
    c[ImGuiCol_FrameBgActive]  = {0.18f, 0.18f, 0.28f, 1.00f};
    c[ImGuiCol_TitleBg]        = {0.07f, 0.07f, 0.11f, 1.00f};
    c[ImGuiCol_TitleBgActive]  = {0.12f, 0.12f, 0.20f, 1.00f};
    c[ImGuiCol_MenuBarBg]      = {0.07f, 0.07f, 0.10f, 1.00f};
    c[ImGuiCol_ScrollbarBg]    = {0.07f, 0.07f, 0.10f, 1.00f};
    c[ImGuiCol_ScrollbarGrab]  = {0.25f, 0.25f, 0.38f, 1.00f};
    c[ImGuiCol_ScrollbarGrabHovered] = {0.35f, 0.35f, 0.52f, 1.00f};
    c[ImGuiCol_ScrollbarGrabActive]  = {0.42f, 0.42f, 0.62f, 1.00f};
    c[ImGuiCol_CheckMark]      = {0.45f, 0.62f, 1.00f, 1.00f};
    c[ImGuiCol_SliderGrab]     = {0.45f, 0.62f, 1.00f, 1.00f};
    c[ImGuiCol_SliderGrabActive] = {0.55f, 0.72f, 1.00f, 1.00f};
    c[ImGuiCol_Button]         = {0.18f, 0.22f, 0.38f, 1.00f};
    c[ImGuiCol_ButtonHovered]  = {0.28f, 0.36f, 0.62f, 1.00f};
    c[ImGuiCol_ButtonActive]   = {0.35f, 0.46f, 0.78f, 1.00f};
    c[ImGuiCol_Header]         = {0.18f, 0.22f, 0.38f, 1.00f};
    c[ImGuiCol_HeaderHovered]  = {0.25f, 0.32f, 0.55f, 1.00f};
    c[ImGuiCol_HeaderActive]   = {0.30f, 0.40f, 0.68f, 1.00f};
    c[ImGuiCol_Separator]      = {0.22f, 0.22f, 0.34f, 1.00f};
    c[ImGuiCol_Tab]            = {0.12f, 0.12f, 0.19f, 1.00f};
    c[ImGuiCol_TabHovered]     = {0.28f, 0.36f, 0.62f, 1.00f};
    c[ImGuiCol_TabActive]      = {0.22f, 0.30f, 0.52f, 1.00f};
    c[ImGuiCol_TabUnfocused]   = {0.10f, 0.10f, 0.16f, 1.00f};
    c[ImGuiCol_TabUnfocusedActive] = {0.16f, 0.22f, 0.38f, 1.00f};
    c[ImGuiCol_Text]           = {0.90f, 0.90f, 0.96f, 1.00f};
    c[ImGuiCol_TextDisabled]   = {0.44f, 0.44f, 0.56f, 1.00f};
    c[ImGuiCol_PlotLines]      = {0.45f, 0.62f, 1.00f, 1.00f};
    c[ImGuiCol_PlotHistogram]  = {0.45f, 0.62f, 1.00f, 1.00f};
}

// ─────────────────────────────────────────────────────────────────────────────
void Editor::BeginFrame() {
    ImGui_ImplNVRHI_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Editor::EndFrame() {
    ImGui::Render();
    nvrhi::CommandListHandle cmdList = m_RHI->GetCurrentCommandList();
    if (cmdList) {
        auto fb = m_RHI->GetCurrentFramebuffer();
        if (fb)
            ImGui_ImplNVRHI_RenderDrawData(ImGui::GetDrawData(), cmdList, fb);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// HUB
// ─────────────────────────────────────────────────────────────────────────────
void Editor::OnHubUpdate(Application* app) { DrawHub(app); }

void Editor::DrawHub(Application* app) {
    ImGuiIO& io = ImGui::GetIO();
    const float W = io.DisplaySize.x;
    const float H = io.DisplaySize.y;

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({W, H});
    ImGui::Begin("##Hub", nullptr,
        ImGuiWindowFlags_NoTitleBar   | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove       | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::SetCursorPos({20, 18});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.48f, 0.68f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.8f);
    ImGui::Text("DoEngine");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::SetCursorPos({W - 130.0f, 24.0f});
    ImGui::TextDisabled("v0.1.0 Alpha");
    ImGui::SetCursorPosY(55.0f);
    ImGui::Separator();

    const float leftW  = 370.0f;
    const float rightW = W - leftW - 40.0f;
    const float panelY = 68.0f;
    const float panelH = H - panelY - 36.0f;

    // Sol: Yeni Proje
    ImGui::SetCursorPos({14, panelY});
    ImGui::BeginChild("##Left", {leftW, panelH}, true);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.76f, 1.0f, 1.0f));
    ImGui::Text("  Yeni Proje Olustur");
    ImGui::PopStyleColor();
    ImGui::Separator(); ImGui::Spacing();
    ImGui::Text("Proje Adi:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##ProjName", m_NewProjectName, sizeof(m_NewProjectName));
    ImGui::Spacing();
    ImGui::Text("Konum:");
    ImGui::SetNextItemWidth(-72.0f);
    ImGui::InputText("##ProjPath", m_NewProjectPath, sizeof(m_NewProjectPath));
    ImGui::SameLine();
    if (ImGui::Button("...")) AddLog("[Hub] Klasor secici: henuz eklenmedi.");
    ImGui::Spacing();
    if (strlen(m_NewProjectName) > 0) {
        std::string preview = std::string(m_NewProjectPath) + "\\" + m_NewProjectName;
        ImGui::TextDisabled("Yol: %s", preview.c_str());
    }
    ImGui::Spacing(); ImGui::Spacing();
    bool canCreate = strlen(m_NewProjectName) > 0 && strlen(m_NewProjectPath) > 0;
    ImGui::PushStyleColor(ImGuiCol_Button,        {0.20f, 0.45f, 0.85f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.30f, 0.56f, 0.95f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.15f, 0.38f, 0.75f, 1.0f});
    if (!canCreate) ImGui::BeginDisabled();
    if (ImGui::Button("  Proje Olustur  ", {-1, 38})) {
        if (ProjectManager::CreateNewProject(m_NewProjectName, m_NewProjectPath)) {
            std::string projPath = std::string(m_NewProjectPath) + "\\" + m_NewProjectName;
            AddLog("[Hub] Proje olusturuldu: " + projPath);
            app->OpenProject(projPath);
        } else {
            AddLog("[Hub] HATA: Proje olusturulamadi!");
        }
    }
    if (!canCreate) ImGui::EndDisabled();
    ImGui::PopStyleColor(3);
    ImGui::EndChild();

    // Sağ: Son Projeler
    ImGui::SetCursorPos({leftW + 26.0f, panelY});
    ImGui::BeginChild("##Right", {rightW, panelH}, true);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.76f, 1.0f, 1.0f));
    ImGui::Text("  Son Projeler");
    ImGui::PopStyleColor();
    ImGui::Separator(); ImGui::Spacing();
    auto recents = ProjectManager::GetRecentProjects();
    if (recents.empty()) {
        ImGui::SetCursorPosY((panelH - 40.0f) * 0.5f);
        const char* t = "Henuz proje yok.";
        ImGui::SetCursorPosX((rightW - ImGui::CalcTextSize(t).x) * 0.5f);
        ImGui::TextDisabled("%s", t);
    } else {
        for (int i = 0; i < (int)recents.size(); i++) {
            const auto& proj = recents[i];
            bool exists = std::filesystem::exists(proj.RootPath);
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Button,
                exists ? ImVec4(0.13f, 0.16f, 0.24f, 1.0f) : ImVec4(0.20f, 0.10f, 0.10f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                exists ? ImVec4(0.20f, 0.26f, 0.40f, 1.0f) : ImVec4(0.30f, 0.14f, 0.14f, 1.0f));
            if (ImGui::Button("##projbtn", {-1, 54}) && exists) {
                app->OpenProject(proj.RootPath);
                AddLog("[Hub] Proje acildi: " + proj.Name);
            }
            ImVec2 bMin = ImGui::GetItemRectMin();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddText({bMin.x+12, bMin.y+6},  IM_COL32(180,205,255,255), proj.Name.c_str());
            dl->AddText({bMin.x+12, bMin.y+28}, IM_COL32(110,115,145,255), proj.RootPath.c_str());
            if (!exists) dl->AddText({bMin.x+rightW-130.0f, bMin.y+8}, IM_COL32(255,80,80,200), "[Bulunamadi]");
            ImGui::PopStyleColor(2);
            ImGui::Spacing();
            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    // Status bar
    ImGui::SetCursorPos({0, H - 30.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.10f, 1.0f));
    ImGui::BeginChild("##Status", {W, 30}, false);
    ImGui::SetCursorPos({10, 6});
    if (!s_LogMessages.empty()) ImGui::TextDisabled("%s", s_LogMessages.back().c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
// EDITOR
// ─────────────────────────────────────────────────────────────────────────────
void Editor::OnUpdate(Scene* scene) {
    ImGuiIO& io = ImGui::GetIO();
    const float W = io.DisplaySize.x;
    const float H = io.DisplaySize.y;
    const float menuH      = 30.0f;
    const float outlinerW  = 240.0f;
    const float inspectorW = 280.0f;
    const float bottomH    = 180.0f;
    const float statusH    = 22.0f;
    const float viewportW  = W - outlinerW - inspectorW;
    const float viewportH  = H - menuH - bottomH - statusH;
    m_ViewportSize[0] = viewportW;
    m_ViewportSize[1] = viewportH;

    // Menu bar
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({W, menuH});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("##MenuWin", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar();
    DrawMenuBar(scene);
    ImGui::End();

    // Outliner
    ImGui::SetNextWindowPos({0, menuH});
    ImGui::SetNextWindowSize({outlinerW, H - menuH - bottomH - statusH});
    ImGui::Begin("Sahne Hiyerarsisi", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    DrawOutliner(scene);
    ImGui::End();

    // Viewport
    ImGui::SetNextWindowPos({outlinerW, menuH});
    ImGui::SetNextWindowSize({viewportW, viewportH});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    DrawViewport();
    ImGui::End();

    // Inspector
    ImGui::SetNextWindowPos({outlinerW + viewportW, menuH});
    ImGui::SetNextWindowSize({inspectorW, H - menuH - bottomH - statusH});
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    DrawInspector(scene);
    ImGui::End();

    // Content Browser + Log
    const float cbW  = W * 0.62f;
    const float logW = W - cbW;
    const float botY = H - bottomH - statusH;
    ImGui::SetNextWindowPos({0, botY});
    ImGui::SetNextWindowSize({cbW, bottomH});
    ImGui::Begin("Icerik Tarayicisi", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    DrawContentBrowser();
    ImGui::End();

    ImGui::SetNextWindowPos({cbW, botY});
    ImGui::SetNextWindowSize({logW, bottomH});
    ImGui::Begin("Log", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    DrawLogPanel();
    ImGui::End();

    // Status bar
    ImGui::SetNextWindowPos({0, H - statusH});
    ImGui::SetNextWindowSize({W, statusH});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.10f, 1.0f));
    ImGui::Begin("##StatusEditor", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::SetCursorPos({8, 3});
    if (!s_LogMessages.empty()) ImGui::TextDisabled("%s", s_LogMessages.back().c_str());
    ImGui::SameLine(W - 150.0f);
    ImGui::TextDisabled("DoEngine v0.1.0");
    ImGui::End();
    ImGui::PopStyleColor();

    if (m_ShowCreateEntityPopup) CreateEntityPopup(scene);
}

// ─────────────────────────────────────────────────────────────────────────────
void Editor::DrawMenuBar(Scene* scene) {
    if (!ImGui::BeginMenuBar()) return;
    if (ImGui::BeginMenu("Dosya")) {
        if (ImGui::MenuItem("Yeni Sahne",     "Ctrl+N")) AddLog("[Editor] Yeni sahne.");
        if (ImGui::MenuItem("Sahneyi Kaydet", "Ctrl+S")) AddLog("[Editor] Sahne kaydedildi.");
        ImGui::Separator();
        if (ImGui::MenuItem("Hub'a Don")) AddLog("[Editor] Hub'a donuluyor.");
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Duzen")) { ImGui::MenuItem("Geri Al", "Ctrl+Z"); ImGui::MenuItem("Yinele", "Ctrl+Y"); ImGui::EndMenu(); }
    if (ImGui::BeginMenu("Build")) { if (ImGui::MenuItem("Projeyi Derle")) AddLog("[Build] Yakindan."); ImGui::EndMenu(); }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetCursorPosX(io.DisplaySize.x * 0.5f - 70.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, {0.15f, 0.48f, 0.22f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.22f, 0.62f, 0.30f, 1.0f});
    if (ImGui::Button(" Oynat ")) AddLog("[Editor] Oyun basladi.");
    ImGui::PopStyleColor(2);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, {0.48f, 0.22f, 0.10f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.62f, 0.30f, 0.14f, 1.0f});
    if (ImGui::Button(" Durdur ")) AddLog("[Editor] Oyun durdu.");
    ImGui::PopStyleColor(2);
    ImGui::EndMenuBar();
}

void Editor::DrawOutliner(Scene* scene) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.24f, 0.40f, 1.0f));
    if (ImGui::SmallButton("+ Entity")) m_ShowCreateEntityPopup = true;
    ImGui::PopStyleColor();
    ImGui::Separator();
    if (!scene) return;
    auto view = scene->View<TagComponent>();
    for (auto entity : view) {
        auto& tag = view.get<TagComponent>(entity);
        bool sel = (m_SelectedEntity == entity);
        ImGuiTreeNodeFlags f = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (sel) f |= ImGuiTreeNodeFlags_Selected;
        ImGui::TreeNodeEx((void*)(intptr_t)(int)entity, f, "%s", tag.Tag.c_str());
        if (ImGui::IsItemClicked()) m_SelectedEntity = entity;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Sil")) { scene->DestroyEntity(entity); if (m_SelectedEntity == entity) m_SelectedEntity = NullEntity; }
            ImGui::EndPopup();
        }
    }
}

void Editor::DrawInspector(Scene* scene) {
    if (!scene || m_SelectedEntity == NullEntity) { ImGui::TextDisabled("Bir entity secin."); return; }
    if (scene->HasComponent<TagComponent>(m_SelectedEntity)) {
        auto& tag = scene->GetComponent<TagComponent>(m_SelectedEntity);
        char buf[256]; strncpy_s(buf, tag.Tag.c_str(), sizeof(buf));
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##TagInput", buf, sizeof(buf))) tag.Tag = buf;
    }
    ImGui::Separator();
    if (scene->HasComponent<TransformComponent>(m_SelectedEntity)) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& tr = scene->GetComponent<TransformComponent>(m_SelectedEntity);
            float pos[3] = {tr.Transform[3][0], tr.Transform[3][1], tr.Transform[3][2]};
            if (ImGui::DragFloat3("Konum", pos, 0.05f)) {
                tr.Transform[3][0]=pos[0]; tr.Transform[3][1]=pos[1]; tr.Transform[3][2]=pos[2];
            }
        }
    }
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    float btnW = 160.0f;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - btnW) * 0.5f);
    if (ImGui::Button("+ Bilesen Ekle", {btnW, 28})) ImGui::OpenPopup("AddComp");
    if (ImGui::BeginPopup("AddComp")) {
        if (ImGui::MenuItem("Transform") && !scene->HasComponent<TransformComponent>(m_SelectedEntity))
            scene->AddComponent<TransformComponent>(m_SelectedEntity);
        ImGui::EndPopup();
    }
}

void Editor::DrawViewport() {
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 pos  = ImGui::GetCursorScreenPos();
    m_ViewportSize[0] = size.x; m_ViewportSize[1] = size.y;
    ImGui::GetWindowDrawList()->AddRectFilled(pos, {pos.x+size.x, pos.y+size.y}, IM_COL32(18,18,26,255));
    const char* hint = "3D Sahne Gorunumu -- Render yakindan";
    ImVec2 ts = ImGui::CalcTextSize(hint);
    ImGui::GetWindowDrawList()->AddText(
        {pos.x+(size.x-ts.x)*0.5f, pos.y+(size.y-ts.y)*0.5f},
        IM_COL32(70,85,115,200), hint);
    ImGui::SetCursorPos({8, 8});
    ImGui::TextDisabled("W: Tasi  E: Dondur  R: Olcek");
}

void Editor::DrawContentBrowser() {
    std::filesystem::path curPath(m_ContentBrowserPath);
    ImGui::TextDisabled("%s", m_ContentBrowserPath);
    ImGui::Separator();
    if (curPath.has_parent_path())
        if (ImGui::SmallButton("^ Ust Klasor"))
            strncpy_s(m_ContentBrowserPath, curPath.parent_path().string().c_str(), sizeof(m_ContentBrowserPath));
    ImGui::Separator();
    std::error_code ec;
    for (auto& entry : std::filesystem::directory_iterator(curPath, ec)) {
        if (ec) break;
        const auto& p = entry.path();
        bool isDir = entry.is_directory(ec);
        std::string name = p.filename().string();
        ImGui::PushID(name.c_str());
        if (isDir) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.76f, 1.0f, 1.0f));
            if (ImGui::Selectable(("[D] " + name).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
                if (ImGui::IsMouseDoubleClicked(0))
                    strncpy_s(m_ContentBrowserPath, p.string().c_str(), sizeof(m_ContentBrowserPath));
            ImGui::PopStyleColor();
        } else {
            std::string ext = p.extension().string();
            ImVec4 col = {0.82f, 0.82f, 0.88f, 1.0f};
            if (ext == ".lua") col = {1.0f, 0.85f, 0.30f, 1.0f};
            if (ext == ".scn") col = {0.5f, 1.0f, 0.70f, 1.0f};
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::Selectable(("    " + name).c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PopID();
    }
}

void Editor::DrawLogPanel() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.14f, 0.10f, 1.0f));
    if (ImGui::SmallButton("Temizle")) s_LogMessages.clear();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Checkbox("Oto-Kaydir", &m_LogAutoScroll);
    ImGui::Separator();
    ImGui::BeginChild("##LogScroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& msg : s_LogMessages) {
        ImVec4 col = {0.78f, 0.78f, 0.86f, 1.0f};
        if (msg.find("HATA") != std::string::npos || msg.find("error") != std::string::npos) col = {1.0f, 0.42f, 0.42f, 1.0f};
        else if (msg.find("warn") != std::string::npos) col = {1.0f, 0.85f, 0.30f, 1.0f};
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(msg.c_str());
        ImGui::PopStyleColor();
    }
    if (m_LogAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

void Editor::CreateEntityPopup(Scene* scene) {
    ImGui::OpenPopup("Entity Olustur##modal");
    if (ImGui::BeginPopupModal("Entity Olustur##modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char eName[128] = "YeniEntity";
        ImGui::Text("Ad:"); ImGui::SetNextItemWidth(240);
        ImGui::InputText("##ename", eName, sizeof(eName));
        ImGui::Spacing();
        if (ImGui::Button("Olustur", {110, 28})) {
            if (scene) { auto e = scene->CreateEntity(eName); scene->AddComponent<TransformComponent>(e); m_SelectedEntity = e; AddLog(std::string("[Editor] Entity: ") + eName); }
            m_ShowCreateEntityPopup = false; ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Iptal", {110, 28})) { m_ShowCreateEntityPopup = false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    } else { m_ShowCreateEntityPopup = false; }
}

} // namespace DoEngine