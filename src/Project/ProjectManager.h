#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace DoEngine {

    struct ProjectInfo {
        std::string Name;
        std::string RootPath;
    };

    class ProjectManager {
    public:
        static bool CreateNewProject(const std::string& name, const std::string& parentPath);
        static bool OpenProject(const std::string& path, ProjectInfo& outInfo);
        
        static std::vector<ProjectInfo> GetRecentProjects();
        static void SaveRecentProjects(const std::vector<ProjectInfo>& projects);
    };

}
