#include "Window.hpp"
#include <stdexcept>

namespace reactor {
    // The constructor initializes GLFW and creates the window.
    Window::Window(int width, int height, std::string title) : m_title(std::move(title)) {
        // Initialize the GLFW library.
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW!");
        }

        // Tell GLFW not to create an OpenGL context, as we are using Vulkan.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Create the window.
        m_window = glfwCreateWindow(width, height, m_title.c_str(), nullptr, nullptr);
        if (!m_window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window!");
        }

        // Store a pointer to this Window instance in the GLFW window's user data.
        // This allows us to retrieve it in the static callback.
        glfwSetWindowUserPointer(m_window, this);

        // Set the static callback function for framebuffer resize events.
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    }

    // The destructor cleans up GLFW resources.
    Window::~Window() {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();
    }

    // Returns the window's framebuffer dimensions as a vk::Extent2D.
    vk::Extent2D Window::getFramebufferSize() const {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        return vk::Extent2D{
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
    }

    // This function handles the creation of the Vulkan surface.
    // It's part of the Window class because the surface is intrinsically linked to the window.
    vk::SurfaceKHR Window::createVulkanSurface(vk::Instance instance) {
        VkSurfaceKHR surface_c;
        // glfwCreateWindowSurface abstracts the platform-specific details of surface creation.
        if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface_c) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
        // The C-style handle is implicitly convertible to the C++ Vulkan-Hpp handle.
        return surface_c;
    }


    // This is the static callback that GLFW invokes when the window is resized.
    void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        // Retrieve the pointer to our Window class instance.
        auto* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (windowInstance) {
            // Set the flag indicating that a resize has occurred.
            windowInstance->m_framebufferResized = true;
        }
    }
}
