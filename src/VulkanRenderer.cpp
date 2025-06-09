#include "VulkanRenderer.hpp"

#include "Window.hpp"

namespace reactor {
VulkanRenderer::VulkanRenderer() {

    // create the window first
    m_window = std::make_unique<Window>(1280, 720, "Reactor");
    m_context = std::make_unique<VulkanContext>(m_window->getNativeWindow());
}

VulkanRenderer::~VulkanRenderer() {

}

    void VulkanRenderer::drawFrame() {

    }


}
