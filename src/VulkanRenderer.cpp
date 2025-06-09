#include "VulkanRenderer.hpp"

namespace reactor {
VulkanRenderer::VulkanRenderer() {
    m_context = std::make_unique<VulkanContext>();
}

VulkanRenderer::~VulkanRenderer() {

}


}