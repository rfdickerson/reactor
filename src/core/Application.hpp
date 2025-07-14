#pragma once

#include "Camera.hpp"
#include "EventManager.hpp"
#include "OrbitController.hpp"
#include "Window.hpp"
#include "../vulkan/VulkanRenderer.hpp"
#include <memory>

namespace reactor {

class Application {
public:
    Application();
    ~Application();

    // Prevent copying
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Allow moving if needed
    Application(Application&&) = default;
    Application& operator=(Application&&) = default;

    // Runs the main application loop. Returns program exit code.
    void run() const;

private:
    void initialize();

    std::unique_ptr<EventManager> m_eventManager;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<OrbitController> m_orbitController;
    std::unique_ptr<VulkanRenderer> m_renderer;

};

} // namespace reactor

