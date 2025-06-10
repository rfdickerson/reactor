#include "VulkanRenderer.hpp"

#include "Window.hpp"

namespace reactor {
VulkanRenderer::VulkanRenderer() {

    // create the window first
    m_window = std::make_unique<Window>(1280, 720, "Reactor");
    m_context = std::make_unique<VulkanContext>(m_window->getNativeWindow());
    m_swapchain = std::make_unique<Swapchain>(
        m_context->device(),
        m_context->physicalDevice(),
        m_context->surface(), *m_window);

    m_frameManager = std::make_unique<FrameManager>(m_context->device(), 0, 2);

}

VulkanRenderer::~VulkanRenderer() {
    // cleanup
}

    void VulkanRenderer::run() {
        while (!m_window->shouldClose()) {
            Window::pollEvents();
            drawFrame();
        }

        m_context->device().waitIdle();
    }


    void VulkanRenderer::drawFrame() {

    }


}
