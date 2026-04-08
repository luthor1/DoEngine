#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace DoEngine {

    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description) {
        std::cerr << "[GLFW ERROR] (" << error << "): " << description << std::endl;
    }

    Window::Window(const WindowProps& props) {
        Init(props);
    }

    Window::~Window() {
        Shutdown();
    }

    void Window::Init(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        if (!s_GLFWInitialized) {
            int success = glfwInit();
            if (!success) {
                std::cerr << "[CRITICAL ERROR] Could not initialize GLFW!" << std::endl;
                return;
            }
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        // We are using NVRHI, so we don't need a default OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
        
        if (!m_Window) {
            std::cerr << "[CRITICAL ERROR] Could not create GLFW window!" << std::endl;
            return;
        }

        glfwSetWindowUserPointer(m_Window, &m_Data);

        // Window Resize Callback
        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;
            // TODO: Notify Renderer about resize
        });

        std::cout << "[INFO] Window created: " << m_Data.Title << " (" << m_Data.Width << "x" << m_Data.Height << ")" << std::endl;
    }

    void Window::Shutdown() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }
        // NOTE: We don't glfwTerminate here, as we might have multiple windows later.
    }

    void Window::OnUpdate() {
        glfwPollEvents();
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

}
