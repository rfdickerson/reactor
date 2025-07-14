#include "Application.hpp"

namespace reactor {

 Application::Application() {
     initialize();
 }

Application::~Application() = default;

void Application::initialize() {
    m_eventManager = std::make_unique<EventManager>();

    m_window = std::make_unique<Window>(1280, 720, "Reactor", *m_eventManager);
    m_camera = std::make_unique<Camera>();
    m_orbitController = std::make_unique<OrbitController>(*m_camera);

    // subscribe controller to input events.
    m_eventManager->subscribe(EventType::MouseMoved, m_orbitController.get());
    m_eventManager->subscribe(EventType::MouseButtonPressed, m_orbitController.get());
    m_eventManager->subscribe(EventType::MouseButtonReleased, m_orbitController.get());

    RendererConfig config{
        .windowWidth = 1280,
        .windowHeight = 720,
        .windowTitle = "Reactor",
        .vertShaderPath = "../resources/shaders/triangle.vert.spv",
        .fragShaderPath = "../resources/shaders/triangle.frag.spv",
        .compositeVertShaderPath = "../resources/shaders/composite.vert.spv",
        .compositeFragShaderPath = "../resources/shaders/composite.frag.spv"
    };

    m_renderer = std::make_unique<VulkanRenderer>(config, *m_window, *m_camera);
}

void Application::run() const {
    while (!m_window->shouldClose()) {
        Window::pollEvents();
        m_renderer->drawFrame();
    }
}


} // reactor