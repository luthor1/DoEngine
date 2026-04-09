#include "ProjectManager.h"
#include <fstream>
#include <iostream>
#include "../Core/Base.h"

namespace DoEngine {

    bool ProjectManager::CreateNewProject(const std::string& name, const std::string& parentPath) {
        std::filesystem::path projectRoot = std::filesystem::path(parentPath) / name;
        
        try {
            if (std::filesystem::exists(projectRoot)) {
                DO_CORE_ERROR("Project directory already exists: {}", projectRoot.string());
                return false;
            }

            std::filesystem::create_directories(projectRoot / "Content");
            std::filesystem::create_directories(projectRoot / "Config");
            std::filesystem::create_directories(projectRoot / "Saved");
            
            // Create .doproj file (simple text for now)
            std::ofstream f(projectRoot / (name + ".doproj"));
            f << "ProjectName: " << name << "\n";
            f << "EngineVersion: 1.0.0\n";
            f.close();
            
            DO_CORE_INFO("Successfully created new project at: {}", projectRoot.string());
            return true;
        } catch (const std::exception& e) {
            DO_CORE_ERROR("Failed to create project: {}", e.what());
            return false;
        }
    }

    bool ProjectManager::OpenProject(const std::string& path, ProjectInfo& outInfo) {
        std::filesystem::path projectPath(path);
        
        if (!std::filesystem::exists(projectPath)) {
            DO_CORE_ERROR("Project path does not exist: {}", path);
            return false;
        }

        // Look for .doproj file
        bool found = false;
        for (const auto& entry : std::filesystem::directory_iterator(projectPath)) {
            if (entry.path().extension() == ".doproj") {
                outInfo.Name = entry.path().stem().string();
                outInfo.RootPath = projectPath.string();
                found = true;
                break;
            }
        }

        if (found) {
            DO_CORE_INFO("Successfully opened project: {}", outInfo.Name);
            return true;
        }

        DO_CORE_ERROR("No .doproj file found in: {}", path);
        return false;
    }

    std::vector<ProjectInfo> ProjectManager::GetRecentProjects() {
        // TODO: Load from a config file (e.g. AppData or engine root)
        return {};
    }

    void ProjectManager::SaveRecentProjects(const std::vector<ProjectInfo>& projects) {
        // TODO: Save to a config file
    }

}
