#pragma once

#include "Base.h"
#include <string>

struct GLFWwindow;

namespace DoEngine {

    struct WindowProps {
        std::string Title;
        uint32 Width;
        uint32 Height;

        WindowProps(const std::string& title = "DoEngine",
                   uint32 width = 1280,
                   uint32 height = 720)
            : Title(title), Width(width), Height(height) {}
    };

    class Window {
    public:
        Window(const WindowProps& props);
        ~Window();

        void OnUpdate();

        uint32 GetWidth() const { return m_Data.Width; }
        uint32 GetHeight() const { return m_Data.Height; }

        bool ShouldClose() const;
        void* GetNativeWindow() const { return m_Window; }

    private:
        void Init(const WindowProps& props);
        void Shutdown();

    private:
        GLFWwindow* m_Window;

        struct WindowData {
            std::string Title;
            uint32 Width, Height;
        };

        WindowData m_Data;
    };

}
