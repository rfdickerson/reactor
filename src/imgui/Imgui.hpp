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
    Imgui(VulkanContext& vulkanContext, Window& window, EventManager& eventManager);
    ~Imgui();

    void createFrame();
    void drawFrame(vk::CommandBuffer commandBuffer);

    [[nodiscard]] float getExposure() const { return m_exposure; }
    [[nodiscard]] float getContrast() const { return m_contrast; }
    [[nodiscard]] float getSaturation() const { return m_saturation; }
    [[nodiscard]] float getFogDensity() const { return m_fogDensity; }

    static vk::DescriptorSet createDescriptorSet(vk::ImageView imageView, vk::Sampler sampler);
    void setSceneDescriptorSet(const vk::DescriptorSet descriptorSet) { m_sceneImguiId = descriptorSet; };

private:

    vk::Device m_device;
    EventManager& m_eventManager;
    vk::DescriptorPool m_descriptorPool;

    vk::DescriptorSet m_sceneImguiId;

    float m_exposure = 1.0f;
    float m_contrast = 1.0f;
    float m_saturation = 1.0f;
    float m_fogDensity = 0.001f;

    void ShowDockspace();
    void ShowSceneView();
    void ShowInspector();
    void ShowConsole();
    void SetupInitialDockLayout(ImGuiID dockspace_id);

};

} // reactor

#endif //IMGUI_HPP
