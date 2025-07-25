#include "VulkanRenderer.hpp"

#include "../core/Uniforms.hpp"
#include "../core/Window.hpp"
#include "VulkanUtils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace reactor {
VulkanRenderer::VulkanRenderer(const RendererConfig& config, Window& window, Camera& camera)
: m_config(config), m_window(window), m_camera(camera) {
    createCoreVulkanObjects();
    createSwapchainAndFrameManager();

    // Setup the Uniform Manager
    m_uniformManager = std::make_unique<UniformManager>(*m_allocator, m_frameManager->getFramesInFlightCount());
    m_uniformManager->registerUBO<SceneUBO>("scene");
    m_uniformManager->registerUBO<CompositeUBO>("composite");

    createPipelineAndDescriptors();
    setupUI();
    createMSAAImage();
    createResolveImages();
    createSceneViewImages();
    createDepthImages();
    createSampler();
    createDescriptorSets();
    createDepthPipelineAndDescriptorSets();

}

void VulkanRenderer::createCoreVulkanObjects() {
    m_context   = std::make_unique<VulkanContext>(m_window.getNativeWindow());
    m_allocator = std::make_unique<Allocator>(m_context->physicalDevice(), m_context->device(),
                                              m_context->instance());
}

void VulkanRenderer::createSwapchainAndFrameManager() {
    m_swapchain = std::make_unique<Swapchain>(m_context->device(), m_context->physicalDevice(),
                                              m_context->surface(), m_window);

    uint32_t swapchainImageCount = m_swapchain->getImageViews().size();
    m_frameManager = std::make_unique<FrameManager>(m_context->device(), *m_allocator, 0, 2,
                                                    swapchainImageCount);

    for (const auto& image : m_swapchain->getImages()) {
        m_imageStateTracker.recordState(image, vk::ImageLayout::eUndefined);
    }
}

void VulkanRenderer::createPipelineAndDescriptors() {
    const std::string vertShaderPath = m_config.vertShaderPath;
    const std::string fragShaderPath = m_config.fragShaderPath;

    const std::vector bindings = {
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1,
                                       vk::ShaderStageFlagBits::eVertex),
    };

    m_descriptorSet = std::make_unique<DescriptorSet>(m_context->device(), 2, bindings);

    const std::vector setLayouts = {m_descriptorSet->getLayout()};

    m_pipeline = std::make_unique<Pipeline>(m_context->device(), vk::Format::eR16G16B16A16Sfloat,
                                            vertShaderPath, fragShaderPath, setLayouts, 4, vk::Format::eD32Sfloat, true);

    const std::vector compositeBindings = {
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment),
    };

    m_compositeDescriptorSet = std::make_unique<DescriptorSet>(m_context->device(), 2, compositeBindings);
    std::vector compositeSetLayouts = {m_compositeDescriptorSet->getLayout()};

    vk::Format swapchainFormat = m_swapchain->getFormat();

    m_compositePipeline = std::make_unique<Pipeline>(
        m_context->device(),
        swapchainFormat,
        m_config.compositeVertShaderPath,
        m_config.compositeFragShaderPath,
        compositeSetLayouts, 1);
}

void VulkanRenderer::handleSwapchainResizing() {
    if (m_window.wasResized()) {
        vk::Extent2D size = m_window.getFramebufferSize();
        while (size.width == 0 || size.height == 0) {
            Window::waitEvents();
            size = m_window.getFramebufferSize();
        }
        m_context->device().waitIdle();
        m_swapchain->recreate();
        m_window.resetResizedFlag();
    }
}

void VulkanRenderer::setupUI() { m_imgui = std::make_unique<Imgui>(*m_context, m_window); }

VulkanRenderer::~VulkanRenderer() {

    m_context->device().waitIdle();

    for (auto i = 0; i < m_frameManager->getFramesInFlightCount(); ++i) {
        m_context->device().destroyImageView(m_msaaColorViews[i]);
        m_context->device().destroyImageView(m_resolveViews[i]);
        m_context->device().destroyImageView(m_sceneViewViews[i]);
        m_context->device().destroyImageView(m_depthViews[i]);
    }
}

void VulkanRenderer::beginCommandBuffer(vk::CommandBuffer cmd) {
    cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
}

void VulkanRenderer::bindDescriptorSets(vk::CommandBuffer cmd) {
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline->getLayout(), 0,
                           m_descriptorSet->getCurrentSet(m_frameManager->getFrameIndex()),
                           nullptr);
}

void VulkanRenderer::drawGeometry(vk::CommandBuffer cmd) {
    //cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->get());
    cmd.draw(36, 1, 0, 0);
}

void VulkanRenderer::renderUI(const vk::CommandBuffer cmd) const {
    m_imgui->createFrame();
    m_imgui->drawFrame(cmd);
}

void VulkanRenderer::endDynamicRendering(vk::CommandBuffer cmd) { cmd.endRendering(); }


void VulkanRenderer::endCommandBuffer(vk::CommandBuffer cmd) { cmd.end(); }

void VulkanRenderer::submitAndPresent(uint32_t imageIndex) {
    m_frameManager->endFrame(m_context->graphicsQueue(), m_context->presentQueue(),
                             m_swapchain->get(), imageIndex);
}

void VulkanRenderer::beginDynamicRendering(
    vk::CommandBuffer cmd,
    vk::ImageView colorImageView,
    vk::ImageView depthImageView,
    vk::Extent2D extent,
    bool clearColor=true,
    bool clearDepth=false)
{
    constexpr vk::ClearValue clearColorValue = vk::ClearColorValue(std::array{0.0f, 0.0f, 0.0f, 1.0f});
    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView   = colorImageView;
    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.loadOp      = clearColor ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
    colorAttachment.storeOp     = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue  = clearColorValue;

    vk::ClearValue depthClearValue{};
    depthClearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderingAttachmentInfo depthAttachment{};
    depthAttachment.imageView   = depthImageView;
    depthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthAttachment.loadOp      = clearDepth ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
    depthAttachment.storeOp     = vk::AttachmentStoreOp::eStore;
    depthAttachment.clearValue  = depthClearValue;

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset    = vk::Offset2D{0, 0};
    renderingInfo.renderArea.extent    = extent;
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments    = &colorAttachment;
    renderingInfo.pDepthAttachment     = &depthAttachment;

    cmd.beginRendering(renderingInfo);
}

void VulkanRenderer::drawFrame() {
    handleSwapchainResizing();

    uint32_t imageIndex;
    if (!m_frameManager->beginFrame(m_swapchain->get(), imageIndex)) {
        return; // Swapchain out-of-date
    }

    const auto             &currentFrame   = m_frameManager->getCurrentFrame();
    const uint32_t          frameIdx       = m_frameManager->getCurrentFrameIndex();
    const vk::CommandBuffer cmd            = currentFrame.commandBuffer;
    const vk::Image         swapchainImage = m_swapchain->getImages()[imageIndex];
    const vk::Extent2D      extent         = m_swapchain->getExtent();

    const auto          width        = m_swapchain->getExtent().width;
    const auto          height       = m_swapchain->getExtent().height;
    const vk::Image     msaaImage    = m_msaaImages[frameIdx]->get();
    const vk::ImageView msaaView     = m_msaaColorViews[frameIdx];
    const vk::Image     resolveImage = m_resolveImages[frameIdx]->get();
    const vk::Image     sceneViewImage = m_sceneViewImages[frameIdx]->get();

    SceneUBO sceneData{};
    const auto aspect = static_cast<float>(width) / static_cast<float>(height);
    sceneData.view = glm::mat4(1.0);
    sceneData.projection = glm::perspective(glm::radians(45.0F), aspect, 0.1F, 100.0F);
    m_uniformManager->update<SceneUBO>(frameIdx, sceneData);

    CompositeUBO compositeData;
    compositeData.uExposure = m_imgui->getExposure();
    compositeData.uContrast = m_imgui->getContrast();
    compositeData.uSaturation = m_imgui->getSaturation();
    m_uniformManager->update<CompositeUBO>(frameIdx, compositeData);

    SceneUBO ubo{};
    ubo.view = m_camera.getView();
    ubo.projection = m_camera.getProjection();

    m_uniformManager->update<SceneUBO>(frameIdx, ubo);

    vk::DescriptorBufferInfo sceneBufferInfo = m_uniformManager->getDescriptorInfo<SceneUBO>(frameIdx);
    vk::WriteDescriptorSet sceneWrite{};
    sceneWrite.dstSet = m_descriptorSet->getCurrentSet(frameIdx);
    sceneWrite.dstBinding = 0;
    sceneWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    sceneWrite.descriptorCount = 1;
    sceneWrite.pBufferInfo = &sceneBufferInfo;

    m_descriptorSet->updateSet({sceneWrite});

    beginCommandBuffer(cmd);

    // get depth image view for this frame
    vk::ImageView depthView = m_depthViews[frameIdx];

    beginDynamicRendering(cmd, nullptr, depthView, extent, false, true);
    utils::setupViewportAndScissor(cmd, extent);
    bindDescriptorSets(cmd);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_depthPipeline->get());
    drawGeometry(cmd);
    endDynamicRendering(cmd);

    // --- 1. Geometry Pass ---
    // Transition the MSAA image so we can render the main scene into it.
    // Its layout was likely UNDEFINED (on first use) or TRANSFER_SRC (from previous frame's resolve).
    m_imageStateTracker.transition(
        cmd,
        msaaImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::PipelineStageFlagBits::eTopOfPipe,          // Source Stage
        vk::PipelineStageFlagBits::eColorAttachmentOutput, // Destination Stage
        {},                                             // Source Access
        vk::AccessFlagBits::eColorAttachmentWrite       // Destination Access
    );

    beginDynamicRendering(cmd, msaaView, depthView, extent, true, false);
    utils::setupViewportAndScissor(cmd, extent);
    bindDescriptorSets(cmd);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->get());
    drawGeometry(cmd);
    endDynamicRendering(cmd);

    // --- 2. MSAA Resolve ---
    // Resolve the multi-sampled image into a standard image for post-processing.
    // vkCmdResolveImage requires the source to be TRANSFER_SRC and destination to be TRANSFER_DST.

    // Transition MSAA image from COLOR_ATTACHMENT to TRANSFER_SRC_OPTIMAL to be read by the resolve command.
    m_imageStateTracker.transition(
        cmd,
        msaaImage,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eTransfer,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eTransferRead
    );

    // Transition the Resolve image to TRANSFER_DST_OPTIMAL to be written to by the resolve command.
    m_imageStateTracker.transition(
        cmd,
        resolveImage,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        vk::AccessFlagBits::eTransferWrite
    );

    utils::resolveMSAAImageTo(cmd, msaaImage, resolveImage, width, height);

    // --- 3. Composite Pass ---
    // This pass reads from the resolved image and writes to the swapchain image.

    // Transition the Resolve image from TRANSFER_DST to SHADER_READ_ONLY so it can be used as a texture.
    m_imageStateTracker.transition(
        cmd,
        resolveImage,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderRead
    );

    // Transition the Swapchain image to COLOR_ATTACHMENT_OPTIMAL so we can render the composite result to it.
    m_imageStateTracker.transition(
        cmd,
        swapchainImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    beginDynamicRendering(cmd, m_sceneViewViews[frameIdx], depthView, extent, true);
    utils::setupViewportAndScissor(cmd, extent);

    vk::DescriptorBufferInfo compositeBufferInfo = m_uniformManager->getDescriptorInfo<CompositeUBO>(frameIdx);

    vk::DescriptorImageInfo imageInfo = {};
    imageInfo.imageView = m_resolveViews[frameIdx];
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.sampler = m_sampler->get();

    std::vector writes = {
        vk::WriteDescriptorSet{
            m_compositeDescriptorSet->getCurrentSet(frameIdx),
            0,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &imageInfo,
            nullptr,
        },
        vk::WriteDescriptorSet{
            m_compositeDescriptorSet->getCurrentSet(frameIdx),
            1,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &compositeBufferInfo,
            nullptr
        }
    };
    m_compositeDescriptorSet->updateSet(writes);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_compositePipeline->get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_compositePipeline->getLayout(), 0, m_compositeDescriptorSet->getCurrentSet(m_frameManager->getFrameIndex()), nullptr);
    cmd.draw(3, 1, 0, 0);
    endDynamicRendering(cmd);

    // -- Prepare sceneView for ImGui
    m_imageStateTracker.transition(
        cmd,
        sceneViewImage,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eShaderRead
    );

    // UI pass
    m_imageStateTracker.transition(
        cmd,
        swapchainImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    // --- 4. UI Pass ---
    // The UI is rendered on top of the composited scene.
    // The swapchain image is already in COLOR_ATTACHMENT_OPTIMAL, so no transition is needed.
    beginDynamicRendering(cmd, m_swapchain->getImageViews()[imageIndex], nullptr, extent, false);

    m_imgui->setSceneDescriptorSet(m_sceneViewImageDescriptorSets[frameIdx]);
    renderUI(cmd);
    endDynamicRendering(cmd);

    // --- 5. Prepare for Presentation ---
    // Transition the swapchain image from COLOR_ATTACHMENT to PRESENT_SRC_KHR for the presentation engine.
    m_imageStateTracker.transition(
        cmd,
        swapchainImage,
        vk::ImageLayout::ePresentSrcKHR,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::AccessFlagBits::eColorAttachmentWrite,
        {}
    );

    endCommandBuffer(cmd);

    submitAndPresent(imageIndex);
}

void VulkanRenderer::createMSAAImage() {
    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e4;

    // set format to HDR capable space
    vk::Format format = vk::Format::eR16G16B16A16Sfloat;
    ;
    constexpr vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment |
                                          vk::ImageUsageFlagBits::eInputAttachment |
                                          vk::ImageUsageFlagBits::eTransferSrc;

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.extent.width  = m_swapchain->getExtent().width;
    imageInfo.extent.height = m_swapchain->getExtent().height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = format;
    imageInfo.tiling        = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage         = usage;
    imageInfo.samples       = msaaSamples;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;

    size_t framesInFlight = m_frameManager->getFramesInFlightCount();

    m_msaaImages.resize(framesInFlight);
    m_msaaColorViews.resize(framesInFlight);

    for (size_t i = 0; i < framesInFlight; ++i) {
        // Create the Image object
        m_msaaImages[i] =
            std::make_unique<Image>(*m_allocator, imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

        m_imageStateTracker.recordState(m_msaaImages[i]->get(), vk::ImageLayout::eUndefined);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image    = m_msaaImages[i]->get(); // Get the VkImage from your new Image object
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format   = format;
        viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        m_msaaColorViews[i] = m_context->device().createImageView(viewInfo);
    }
}

void VulkanRenderer::createResolveImages() {
    vk::Format   format = vk::Format::eR16G16B16A16Sfloat; // Match your swapchain/attachment format
    vk::Extent2D extent = m_swapchain->getExtent();

    size_t framesInFlight = m_frameManager->getFramesInFlightCount();
    m_resolveImages.resize(framesInFlight);
    m_resolveViews.resize(framesInFlight);

    for (size_t i = 0; i < framesInFlight; ++i) {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType     = vk::ImageType::e2D;
        imageInfo.extent.width  = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = format;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage         = vk::ImageUsageFlagBits::eColorAttachment |
                          vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        imageInfo.samples     = vk::SampleCountFlagBits::e1; // Resolve is NOT multisampled!
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        // Create the Image (assume you have your own Image wrapper e.g. using VMA)
        m_resolveImages[i] =
            std::make_unique<Image>(*m_allocator, imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

        m_imageStateTracker.recordState(m_resolveImages[i]->get(), vk::ImageLayout::eUndefined);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image                           = m_resolveImages[i]->get();
        viewInfo.viewType                        = vk::ImageViewType::e2D;
        viewInfo.format                          = format;
        viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        m_resolveViews[i] = m_context->device().createImageView(viewInfo);
    }
}
void VulkanRenderer::createSceneViewImages() {
    vk::Format   format = m_swapchain->getFormat();
    vk::Extent2D extent = m_swapchain->getExtent();

    size_t framesInFlight = m_frameManager->getFramesInFlightCount();

    // destroy old resources if recreating
    for (size_t i = 0; i < m_sceneViewViews.size(); ++i) {
        m_context->device().destroyImageView(m_sceneViewViews[i]);
    }
    m_sceneViewImages.clear();
    m_sceneViewViews.clear();

    m_sceneViewImages.resize(framesInFlight);
    m_sceneViewViews.resize(framesInFlight);

    for (size_t i = 0; i < framesInFlight; ++i) {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType     = vk::ImageType::e2D;
        imageInfo.extent.width  = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = format;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage         = vk::ImageUsageFlagBits::eColorAttachment |
            vk::ImageUsageFlagBits::eSampled |
            vk::ImageUsageFlagBits::eTransferSrc;
        imageInfo.samples     = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        m_sceneViewImages[i] = std::make_unique<Image>(*m_allocator, imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

        m_imageStateTracker.recordState(m_sceneViewImages[i]->get(), vk::ImageLayout::eUndefined);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image                           = m_sceneViewImages[i]->get();
        viewInfo.viewType                        = vk::ImageViewType::e2D;
        viewInfo.format                          = format;
        viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        m_sceneViewViews[i] = m_context->device().createImageView(viewInfo);
    }
}

void VulkanRenderer::createSampler() {
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter        = vk::Filter::eLinear;
    samplerInfo.minFilter        = vk::Filter::eLinear;
    samplerInfo.mipmapMode       = vk::SamplerMipmapMode::eLinear;
    samplerInfo.addressModeU     = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV     = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW     = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.mipLodBias       = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy    = 1.0f;
    samplerInfo.compareEnable    = VK_FALSE;
    samplerInfo.compareOp        = vk::CompareOp::eAlways;
    samplerInfo.minLod           = 0.0f;
    samplerInfo.maxLod           = 0.0f;
    samplerInfo.borderColor      = vk::BorderColor::eFloatOpaqueWhite;

    m_sampler = std::make_unique<Sampler>(m_context->device(), samplerInfo);
}
void VulkanRenderer::createDescriptorSets() {

    const auto framesInFlight = m_frameManager->getFramesInFlightCount();
    m_sceneViewImageDescriptorSets.resize(framesInFlight);
    for (int i = 0; i < framesInFlight; ++i) {
        m_sceneViewImageDescriptorSets[i] =
            m_imgui->createDescriptorSet(m_sceneViewViews[i], m_sampler->get());
    }
}
void VulkanRenderer::createDepthImages() {
    vk::Format   format         = vk::Format::eD32Sfloat; // Common depth format
    vk::Extent2D extent         = m_swapchain->getExtent();
    size_t       framesInFlight = m_frameManager->getFramesInFlightCount();

    for (size_t i = 0; i < m_depthImages.size(); ++i) {
        m_context->device().destroyImageView(m_depthViews[i]);
    }
    m_depthImages.clear();
    m_depthViews.clear();

    m_depthImages.resize(framesInFlight);
    m_depthViews.resize(framesInFlight);
    for (size_t i = 0; i < framesInFlight; ++i) {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType     = vk::ImageType::e2D;
        imageInfo.extent.width  = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = format;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage =
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
        imageInfo.samples     = vk::SampleCountFlagBits::e4;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        m_depthImages[i] =
            std::make_unique<Image>(*m_allocator, imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);
        m_imageStateTracker.recordState(m_depthImages[i]->get(), vk::ImageLayout::eUndefined);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image                           = m_depthImages[i]->get();
        viewInfo.viewType                        = vk::ImageViewType::e2D;
        viewInfo.format                          = format;
        viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eDepth;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        m_depthViews[i] = m_context->device().createImageView(viewInfo);
    }
}
void VulkanRenderer::createDepthPipelineAndDescriptorSets() {

    const std::string depthVertexShaderPath = m_config.vertShaderPath;
    const std::string emptyFragShaderPath = ""; // no fragment shader

    std::vector<vk::DescriptorSetLayout> setLayouts = {
        m_descriptorSet->getLayout()};

    m_depthPipeline = std::make_unique<Pipeline>(
        m_context->device(),
        vk::Format::eUndefined,
        depthVertexShaderPath,
        emptyFragShaderPath,
        setLayouts,
        4,
        vk::Format::eD32Sfloat,
        true
        );


}

} // namespace reactor
