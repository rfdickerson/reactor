//
// Created by rfdic on 6/9/2025.
//

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>
#include <vulkan/vulkan.hpp>
#include "EventManager.hpp"

namespace reactor {


    class Window {
    public:
        Window(int width, int height, std::string title, EventManager& eventManager);
        ~Window();

        // Prevent copying and moving to ensure a single owner for the window resource.
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        // Checks if the user has requested the window to close (e.g., by clicking the 'X' button).
        bool shouldClose() const { return glfwWindowShouldClose(m_window); }

        // Processes all pending events in the window's event queue.
        static void pollEvents() { glfwPollEvents(); }

        // Puts the current thread to sleep until at least one event is available.
        // Useful for preventing the app from spinning at 100% CPU when minimized.
        static void waitEvents() { glfwWaitEvents(); }

        // Gets the underlying native window handle.
        GLFWwindow* getNativeWindow() const { return m_window; }

        // Gets the current size of the framebuffer in pixels.
        // This can be different from the window size on high-DPI displays.
        vk::Extent2D getFramebufferSize() const;

        // Creates a Vulkan surface (the connection between Vulkan and the window system).
        vk::SurfaceKHR createVulkanSurface(vk::Instance instance);

        // -- Resize Handling --
        bool wasResized() const { return m_framebufferResized; }
        void resetResizedFlag() { m_framebufferResized = false; }


    private:
        // This is the static callback function that GLFW will call on resize events.
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseButtonCallback(GLFWwindow* window, double xpos, double ypos);

        GLFWwindow* m_window = nullptr;
        EventManager& m_eventManager;
        int m_width;
        int m_height;
        std::string m_title;
        bool m_framebufferResized = false;
    };

}

#endif //WINDOW_HPP
