//
// Created by rfdic on 6/16/2025.
//

#ifndef IMGUI_HPP
#define IMGUI_HPP
#include "VulkanContext.hpp"
#include "Window.hpp"

namespace reactor {

class Imgui {
public:
    Imgui(VulkanContext& vulkanContext, Window& window);
    ~Imgui();

    void createFrame();
    void drawFrame(vk::CommandBuffer commandBuffer);

private:

    vk::Device m_device;
    vk::DescriptorPool m_descriptorPool;

};

} // reactor

#endif //IMGUI_HPP
