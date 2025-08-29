#include "Application.hpp"

namespace reactor {

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

 Application::Application() {
     initialize();
 }

Application::~Application() = default;

void Application::initialize() {
    m_eventManager = std::make_unique<EventManager>();

    m_window = std::make_unique<Window>(WINDOW_WIDTH, WINDOW_HEIGHT, "Reactor", *m_eventManager);
    m_camera = std::make_unique<Camera>();
    m_orbitController = std::make_unique<OrbitController>(*m_camera);
    m_orbitController->setSimCityView(20.0f, 45.0f);

    // subscribe controller to input events.
    m_eventManager->subscribe(EventType::MouseMoved, m_orbitController.get());
    m_eventManager->subscribe(EventType::MouseButtonPressed, m_orbitController.get());
    m_eventManager->subscribe(EventType::MouseButtonReleased, m_orbitController.get());

    RendererConfig config{
        .windowWidth = WINDOW_WIDTH,
        .windowHeight = WINDOW_HEIGHT,
        .windowTitle = "Reactor",
        .vertShaderPath = "../resources/shaders/triangle-slang.vert.spv",
        .fragShaderPath = "../resources/shaders/triangle-slang.frag.spv",
        .compositeVertShaderPath = "../resources/shaders/composite-slang.vert.spv",
        .compositeFragShaderPath = "../resources/shaders/composite-slang.frag.spv"
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