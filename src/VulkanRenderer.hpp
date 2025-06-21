//
// Created by Robert F. Dickerson on 6/9/25.
//

#ifndef VULKANRENDERER_HPP
#define VULKANRENDERER_HPP
#include <memory>

#include "Allocator.hpp"
#include "DescriptorSet.hpp"
#include "VulkanContext.hpp"
#include "Window.hpp"
#include "Swapchain.hpp"
#include "FrameManager.hpp"
#include "Imgui.hpp"
#include "Pipeline.hpp"
#include "Image.hpp"

namespace reactor {

    struct RendererConfig {
        uint32_t windowWidth;
        uint32_t windowHeight;
        std::string windowTitle;
        std::string vertShaderPath;
        std::string fragShaderPath;
    };

class VulkanRenderer {
public:
    VulkanRenderer(const RendererConfig& config);
    ~VulkanRenderer();

    void run();

    void drawFrame();

private:
    RendererConfig m_config{};
    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Allocator> m_allocator;
    std::unique_ptr<FrameManager> m_frameManager;
    std::unique_ptr<DescriptorSet> m_descriptorSet;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<Imgui> m_imgui;
    std::unique_ptr<Image> m_msaaImage;

    vk::ImageView m_msaaImageView;

    void createCoreVulkanObjects();
    void createSwapchainAndFrameManager();
    void createPipelineAndDescriptors();
    void setupUI();
    void createMSAAImage();

    void handleSwapchainResizing();
    void beginCommandBuffer(vk::CommandBuffer cmd);
    void prepareImageForRendering(vk::CommandBuffer cmd, vk::Image image);
    void beginDynamicRendering(vk::CommandBuffer cmd, vk::ImageView imageView, vk::Extent2D extent);
    void setupViewportAndScissor(vk::CommandBuffer cmd, vk::Extent2D extent);
    void updateUniformBuffer(Buffer* uniformBuffer);
    void bindDescriptorSets(vk::CommandBuffer cmd);
    void drawGeometry(vk::CommandBuffer cmd);
    void renderUI(vk::CommandBuffer cmd);
    void endDynamicRendering(vk::CommandBuffer cmd);
    void prepareImageForPresent(vk::CommandBuffer cmd, vk::Image image);
    void endCommandBuffer(vk::CommandBuffer cmd);
    void submitAndPresent(uint32_t imageIndex);
};

}

#endif //VULKANRENDERER_HPP
