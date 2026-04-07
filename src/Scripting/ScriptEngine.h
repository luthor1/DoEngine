#pragma once

#include "../Core/Base.h"
#include <string>
#include <vector>

namespace DoEngine {

    class ScriptEngine {
    public:
        static void Initialize();
        static void Shutdown();

        // Load a C# assembly
        static bool LoadAssembly(const std::string& path);
        
        // Execute a method (placeholder for now)
        static void ExecuteMethod(const std::string& className, const std::string& methodName);

        // Update loop (for C# scripts)
        static void OnUpdate(float deltaTime);

    private:
        // CoreCLR host data would go here
    };

}
