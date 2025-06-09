//
// Created by Robert F. Dickerson on 6/9/25.
//

#ifndef VULKANRENDERER_HPP
#define VULKANRENDERER_HPP
#include <memory>

#include "VulkanContext.hpp"
#include "Window.hpp"

namespace reactor {

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    void drawFrame();

private:
    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<Window> m_window;
};

}

#endif //VULKANRENDERER_HPP
