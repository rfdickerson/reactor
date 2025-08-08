#pragma once

#include <memory>

#include "../core/Camera.hpp"
#include "../core/Uniforms.hpp"
#include "../core/Window.hpp"
#include "../imgui/Imgui.hpp"
#include "Allocator.hpp"
#include "DescriptorSet.hpp"
#include "FrameManager.hpp"
#include "Image.hpp"
#include "ImageStateTracker.h"
#include "Mesh.hpp"
#include "MeshGenerators.hpp"
#include "Pipeline.hpp"
#include "Sampler.hpp"
#include "ShadowMapping.hpp"
#include "Swapchain.hpp"
#include "UniformManager.hpp"
#include "VulkanContext.hpp"

namespace reactor
{

struct RendererConfig
{
    uint32_t windowWidth;
    uint32_t windowHeight;
    std::string windowTitle;
    std::string vertShaderPath;
    std::string fragShaderPath;
    std::string compositeVertShaderPath;
    std::string compositeFragShaderPath;
};

struct RenderObject
{
    std::shared_ptr<Mesh> mesh;
    glm::mat4 transform = glm::mat4(1.0f);
};

class VulkanRenderer
{
public:
    VulkanRenderer(const RendererConfig& config, Window& window, Camera& camera);
    ~VulkanRenderer();

    void drawFrame();

    vk::Device device() const;
    Allocator& allocator();
    vk::DescriptorPool descriptorPool() const;

private:
    const RendererConfig& m_config;
    Window& m_window;
    Camera& m_camera;

    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Allocator> m_allocator;
    std::unique_ptr<FrameManager> m_frameManager;
    std::unique_ptr<DescriptorSet> m_descriptorSet;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<Pipeline> m_compositePipeline;
    std::unique_ptr<Pipeline> m_depthPipeline;
    std::unique_ptr<DescriptorSet> m_compositeDescriptorSet;
    std::unique_ptr<Sampler> m_sampler;
    std::unique_ptr<UniformManager> m_uniformManager;
    std::unique_ptr<Imgui> m_imgui;
    std::unique_ptr<ShadowMapping> m_shadowMapping;

    ImageStateTracker m_imageStateTracker;

    std::vector<std::unique_ptr<Image>> m_msaaImages;
    std::vector<vk::ImageView> m_msaaColorViews;

    std::vector<std::unique_ptr<Image>> m_resolveImages;
    std::vector<vk::ImageView> m_resolveViews;
    std::vector<std::unique_ptr<Image>> m_sceneViewImages;
    std::vector<vk::ImageView> m_sceneViewViews;
    std::vector<vk::DescriptorSet> m_sceneViewImageDescriptorSets;

    std::vector<std::unique_ptr<Image>> m_depthImages;
    std::vector<vk::ImageView> m_depthViews;
    std::vector<vk::DescriptorSet> m_depthImageDescriptorSets;

    // storage for resolved depth
    std::vector<std::unique_ptr<Image>> m_depthResolveImages;
    std::vector<vk::ImageView> m_depthResolveViews;

    DirectionalLightUBO m_light;
    std::vector<RenderObject> m_objects;

    vk::DescriptorPool m_descriptorPool;

    void createCoreVulkanObjects();
    void createSwapchainAndFrameManager();
    void createPipelineAndDescriptors();
    void setupUI(std::shared_ptr<class ImGuiConsoleSink> consoleSink);
    void createMSAAImage();
    void createResolveImages();
    void createSceneViewImages();
    void createSampler();
    void createDescriptorSets();
    void createDepthImages();
    void createDepthPipelineAndDescriptorSets();
    void initScene();
    void createDescriptorPool();

    void handleSwapchainResizing();
    void beginCommandBuffer(vk::CommandBuffer cmd);
    void beginDynamicRendering(vk::CommandBuffer cmd, vk::ImageView colorImageView, vk::ImageView depthImageView, vk::Extent2D extent, bool clearColor, bool clearDepth);
    void bindDescriptorSets(vk::CommandBuffer cmd);
    void drawGeometry(vk::CommandBuffer cmd);
    void renderUI(vk::CommandBuffer cmd) const;
    static void endDynamicRendering(vk::CommandBuffer cmd);
    static void endCommandBuffer(vk::CommandBuffer cmd);
    void submitAndPresent(uint32_t imageIndex);
};

} // namespace reactor
