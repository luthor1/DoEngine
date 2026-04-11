#include "ProjectManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "../Core/Base.h"

namespace DoEngine {

// ─────────────────────────────────────────────────────────────────────────────
// Yardımcı: AppData/DoEngine dizinindeki recent_projects.json yolu
// ─────────────────────────────────────────────────────────────────────────────
static std::filesystem::path GetRecentProjectsPath() {
    const char* appData = std::getenv("APPDATA");
    if (!appData) appData = ".";
    std::filesystem::path dir = std::filesystem::path(appData) / "DoEngine";
    std::filesystem::create_directories(dir);
    return dir / "recent_projects.json";
}

// ─────────────────────────────────────────────────────────────────────────────
// Minimal JSON yardımcıları (harici bağımlılık olmadan)
// ─────────────────────────────────────────────────────────────────────────────
static std::string EscapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '"') out += "\\\"";
        else out += c;
    }
    return out;
}

static std::string ExtractJsonField(const std::string& json,
                                     const std::string& key) {
    std::string search = "\"" + key + "\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    auto end = json.find('"', pos + 1);
    // Handle escaped quotes
    while (end != std::string::npos && json[end - 1] == '\\')
        end = json.find('"', end + 1);
    if (end == std::string::npos) return "";
    std::string val = json.substr(pos + 1, end - pos - 1);
    // Unescape
    std::string result;
    for (size_t i = 0; i < val.size(); i++) {
        if (val[i] == '\\' && i + 1 < val.size()) {
            if (val[i+1] == '\\') { result += '\\'; i++; }
            else if (val[i+1] == '"') { result += '"'; i++; }
            else result += val[i];
        } else result += val[i];
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// CreateNewProject
// ─────────────────────────────────────────────────────────────────────────────
bool ProjectManager::CreateNewProject(const std::string& name,
                                       const std::string& parentPath) {
    std::filesystem::path projectRoot = std::filesystem::path(parentPath) / name;

    try {
        if (std::filesystem::exists(projectRoot)) {
            DO_CORE_ERROR("Project directory already exists: {}", projectRoot.string());
            return false;
        }

        // Klasör yapısı
        std::filesystem::create_directories(projectRoot / "Content");
        std::filesystem::create_directories(projectRoot / "Config");
        std::filesystem::create_directories(projectRoot / "Saved");
        std::filesystem::create_directories(projectRoot / "Scripts");
        std::filesystem::create_directories(projectRoot / "Scenes");

        // .doproj JSON dosyası
        std::ofstream f(projectRoot / (name + ".doproj"));
        f << "{\n";
        f << "  \"name\": \"" << EscapeJson(name) << "\",\n";
        f << "  \"engine_version\": \"0.1.0\",\n";
        f << "  \"default_scene\": \"Scenes/Main.scn\",\n";
        f << "  \"scripting\": \"lua\"\n";
        f << "}\n";
        f.close();

        // Örnek Lua scripti
        std::ofstream ls(projectRoot / "Scripts" / "Player.lua");
        ls << "-- DoEngine Lua Script\n";
        ls << "-- Entity: Player\n\n";
        ls << "function OnStart()\n";
        ls << "    print(\"Oyun başladı!\")\n";
        ls << "end\n\n";
        ls << "function OnUpdate(dt)\n";
        ls << "    -- Hareket mantığı buraya\n";
        ls << "end\n\n";
        ls << "function OnDestroy()\n";
        ls << "    print(\"Entity yok edildi.\")\n";
        ls << "end\n";
        ls.close();

        // Recent projects listesine ekle
        auto recents = GetRecentProjects();
        ProjectInfo info;
        info.Name     = name;
        info.RootPath = projectRoot.string();
        // Duplicate kontrolü
        bool dup = false;
        for (auto& r : recents) { if (r.RootPath == info.RootPath) { dup = true; break; } }
        if (!dup) {
            recents.insert(recents.begin(), info);
            if (recents.size() > 10) recents.resize(10);
            SaveRecentProjects(recents);
        }

        DO_CORE_INFO("Successfully created new project at: {}", projectRoot.string());
        return true;
    } catch (const std::exception& e) {
        DO_CORE_ERROR("Failed to create project: {}", e.what());
        return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OpenProject
// ─────────────────────────────────────────────────────────────────────────────
bool ProjectManager::OpenProject(const std::string& path, ProjectInfo& outInfo) {
    std::filesystem::path projectPath(path);

    if (!std::filesystem::exists(projectPath)) {
        DO_CORE_ERROR("Project path does not exist: {}", path);
        return false;
    }

    // .doproj dosyasını bul
    bool found = false;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(projectPath, ec)) {
        if (entry.path().extension() == ".doproj") {
            outInfo.Name     = entry.path().stem().string();
            outInfo.RootPath = projectPath.string();
            found = true;
            break;
        }
    }

    if (!found) {
        DO_CORE_ERROR("No .doproj file found in: {}", path);
        return false;
    }

    // Recent projects listesine ekle/taşı
    auto recents = GetRecentProjects();
    // Varsa çıkar
    recents.erase(std::remove_if(recents.begin(), recents.end(),
        [&](const ProjectInfo& p) { return p.RootPath == outInfo.RootPath; }),
        recents.end());
    recents.insert(recents.begin(), outInfo);
    if (recents.size() > 10) recents.resize(10);
    SaveRecentProjects(recents);

    DO_CORE_INFO("Successfully opened project: {}", outInfo.Name);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetRecentProjects — JSON'dan oku
// ─────────────────────────────────────────────────────────────────────────────
std::vector<ProjectInfo> ProjectManager::GetRecentProjects() {
    std::vector<ProjectInfo> result;
    auto jsonPath = GetRecentProjectsPath();

    if (!std::filesystem::exists(jsonPath)) return result;

    std::ifstream f(jsonPath);
    if (!f.is_open()) return result;

    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    f.close();

    // Basit array parser: "[{...},{...}]"
    size_t pos = 0;
    while (true) {
        auto open = content.find('{', pos);
        if (open == std::string::npos) break;
        auto close = content.find('}', open);
        if (close == std::string::npos) break;

        std::string obj = content.substr(open, close - open + 1);
        ProjectInfo info;
        info.Name     = ExtractJsonField(obj, "name");
        info.RootPath = ExtractJsonField(obj, "path");

        if (!info.Name.empty() && !info.RootPath.empty())
            result.push_back(info);

        pos = close + 1;
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// SaveRecentProjects — JSON'a yaz
// ─────────────────────────────────────────────────────────────────────────────
void ProjectManager::SaveRecentProjects(const std::vector<ProjectInfo>& projects) {
    auto jsonPath = GetRecentProjectsPath();
    std::ofstream f(jsonPath);
    if (!f.is_open()) {
        DO_CORE_ERROR("Could not save recent_projects.json");
        return;
    }

    f << "[\n";
    for (size_t i = 0; i < projects.size(); i++) {
        f << "  {\n";
        f << "    \"name\": \"" << EscapeJson(projects[i].Name) << "\",\n";
        f << "    \"path\": \"" << EscapeJson(projects[i].RootPath) << "\"\n";
        f << "  }";
        if (i + 1 < projects.size()) f << ",";
        f << "\n";
    }
    f << "]\n";
}

} // namespace DoEngine
