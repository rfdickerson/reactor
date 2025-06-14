//
// Created by Robert F. Dickerson on 6/9/25.
//

#ifndef VULKANRENDERER_HPP
#define VULKANRENDERER_HPP
#include <memory>

#include "DescriptorSet.hpp"
#include "VulkanContext.hpp"
#include "Window.hpp"
#include "Swapchain.hpp"
#include "FrameManager.hpp"
#include "Pipeline.hpp"

namespace reactor {

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    void run();

    void drawFrame();

private:
    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<FrameManager> m_frameManager;
    std::unique_ptr<DescriptorSet> m_descriptorSet;
    std::unique_ptr<Pipeline> m_pipeline;
};

}

#endif //VULKANRENDERER_HPP
