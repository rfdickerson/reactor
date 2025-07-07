//
// Created by Robert F. Dickerson on 6/9/25.
//

#ifndef VULKANRENDERER_HPP
#define VULKANRENDERER_HPP
#include <memory>

#include "../core/Camera.hpp"
#include "../core/EventManager.hpp"
#include "../core/OrbitController.hpp"
#include "../core/Window.hpp"
#include "../imgui/Imgui.hpp"
#include "Allocator.hpp"
#include "DescriptorSet.hpp"
#include "FrameManager.hpp"
#include "Image.hpp"
#include "ImageStateTracker.h"
#include "Pipeline.hpp"
#include "Sampler.hpp"
#include "Swapchain.hpp"
#include "UniformManager.hpp"
#include "VulkanContext.hpp"

namespace reactor {

    struct RendererConfig {
        uint32_t windowWidth;
        uint32_t windowHeight;
        std::string windowTitle;
        std::string vertShaderPath;
        std::string fragShaderPath;
        std::string compositeVertShaderPath;
        std::string compositeFragShaderPath;
    };



class VulkanRenderer {
public:
    explicit VulkanRenderer(const RendererConfig& config);
    ~VulkanRenderer();

    void run();

    void drawFrame();

private:
    RendererConfig m_config{};
    std::unique_ptr<EventManager> m_eventManager;
    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Allocator> m_allocator;
    std::unique_ptr<FrameManager> m_frameManager;
    std::unique_ptr<DescriptorSet> m_descriptorSet;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<Pipeline> m_compositePipeline;
    std::unique_ptr<DescriptorSet> m_compositeDescriptorSet;
    std::unique_ptr<Sampler> m_sampler;
    std::unique_ptr<UniformManager> m_uniformManager;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<OrbitController> m_orbitController;

    std::unique_ptr<Imgui> m_imgui;

    ImageStateTracker m_imageStateTracker;

    std::vector<std::unique_ptr<Image>> m_msaaImages;
    std::vector<vk::ImageView> m_msaaColorViews;

    std::vector<std::unique_ptr<Image>> m_resolveImages;
    std::vector<vk::ImageView> m_resolveViews;

    void createCoreVulkanObjects();
    void createSwapchainAndFrameManager();
    void createPipelineAndDescriptors();
    void setupUI();
    void createMSAAImage();
    void createResolveImages();
    void createSampler();

    void handleSwapchainResizing();
    void beginCommandBuffer(vk::CommandBuffer cmd);
    void beginDynamicRendering(vk::CommandBuffer cmd, vk::ImageView imageView, vk::Extent2D extent, bool clear);
    void updateUniformBuffer(Buffer* uniformBuffer);
    void bindDescriptorSets(vk::CommandBuffer cmd);
    void drawGeometry(vk::CommandBuffer cmd);
    void renderUI(vk::CommandBuffer cmd);
    void endDynamicRendering(vk::CommandBuffer cmd);
    void endCommandBuffer(vk::CommandBuffer cmd);
    void submitAndPresent(uint32_t imageIndex);


};

}

#endif //VULKANRENDERER_HPP
