//
// Created by rfdic on 6/16/2025.
//

#ifndef IMGUI_HPP
#define IMGUI_HPP
#include "../core/Window.hpp"
#include "../vulkan/VulkanContext.hpp"

#include <imgui.h>

namespace reactor {

class Imgui {
public:
    Imgui(VulkanContext& vulkanContext, Window& window);
    ~Imgui();

    void createFrame();
    void drawFrame(vk::CommandBuffer commandBuffer);

    float getExposure() const { return m_exposure; }
    float getContrast() const { return m_contrast; }
    float getSaturation() const { return m_saturation; }

    void SetSceneImage(vk::ImageView imageView, vk::Sampler sampler);

private:

    vk::Device m_device;
    vk::DescriptorPool m_descriptorPool;

    vk::DescriptorSet m_sceneImguiId;

    float m_exposure = 1.0f;
    float m_contrast = 1.0f;
    float m_saturation = 1.0f;

    void ShowDockspace();
    void ShowSceneView();
    void ShowInspector();
    void ShowConsole();
    void SetupInitialDockLayout(ImGuiID dockspace_id);

};

} // reactor

#endif //IMGUI_HPP
