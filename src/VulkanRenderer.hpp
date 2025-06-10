//
// Created by Robert F. Dickerson on 6/9/25.
//

#ifndef VULKANRENDERER_HPP
#define VULKANRENDERER_HPP
#include <memory>

#include "VulkanContext.hpp"
#include "Window.hpp"
#include "Swapchain.hpp"
#include "FrameManager.hpp"

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
};

}

#endif //VULKANRENDERER_HPP
