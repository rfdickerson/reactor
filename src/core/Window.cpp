#include "Window.hpp"
#include <stdexcept>

namespace reactor {
    // The constructor initializes GLFW and creates the window.
    Window::Window(int width, int height, std::string title, EventManager& eventManager) :
m_width(width), m_height(height), m_title(title), m_eventManager(eventManager) {
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

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

        glfwSetKeyCallback(m_window, keyCallback);
        glfwSetCursorPosCallback(m_window, cursorPosCallback);
        glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
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
    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        // Retrieve the pointer to our Window class instance.
        auto *windowInstance = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (windowInstance) {
            // Set the flag indicating that a resize has occurred.
            windowInstance->m_framebufferResized = true;
        }
    }
    void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!windowInstance) return;

        Event event{};
        if (action == GLFW_PRESS) {
            event.type = EventType::KeyPressed;
            event.keyboard.key = key;
            windowInstance->m_eventManager.post(event);
        } else if (action == GLFW_RELEASE) {
            event.type = EventType::KeyReleased;
            event.keyboard.key = key;
            windowInstance->m_eventManager.post(event);
        }

    }

    void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        auto* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!windowInstance) return;

        Event event{};
        if (action == GLFW_PRESS) {
            event.type = EventType::MouseButtonPressed;
            event.mouseButton.button = button;
            windowInstance->m_eventManager.post(event);
        } else if (action == GLFW_RELEASE) {
            event.type = EventType::MouseButtonReleased;
            event.mouseButton.button = button;
        }

        windowInstance->m_eventManager.post(event);
    }

    void Window::cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
        auto* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!windowInstance) return;

        Event event{};
        event.type = EventType::MouseMoved;
        event.mouseMove.x = xpos;
        event.mouseMove.y = ypos;
        windowInstance->m_eventManager.post(event);

    }
    } // namespace reactor
