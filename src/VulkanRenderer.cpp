#include "VulkanRenderer.hpp"

#include "Buffer.hpp"
#include "Window.hpp"
#include "VulkanUtils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

namespace reactor {
    VulkanRenderer::VulkanRenderer(const RendererConfig& config) : m_config(config) {
        createCoreVulkanObjects();
        createSwapchainAndFrameManager();
        createPipelineAndDescriptors();
        setupUI();
        createMSAAImage();
        createResolveImages();
    }

    void VulkanRenderer::createCoreVulkanObjects() {
        m_window = std::make_unique<Window>(
            static_cast<int>(m_config.windowWidth),
            static_cast<int>(m_config.windowHeight),
            m_config.windowTitle);
        m_context = std::make_unique<VulkanContext>(m_window->getNativeWindow());
        m_allocator = std::make_unique<Allocator>( m_context->physicalDevice(), m_context->device(), m_context->instance());
    }

    void VulkanRenderer::createSwapchainAndFrameManager() {
        m_swapchain = std::make_unique<Swapchain>(
            m_context->device(),
            m_context->physicalDevice(),
            m_context->surface(), *m_window);

        uint32_t swapchainImageCount = m_swapchain->getImageViews().size();
        m_frameManager = std::make_unique<FrameManager>(m_context->device(), *m_allocator, 0, 2, swapchainImageCount);
    }

    void VulkanRenderer::createPipelineAndDescriptors() {
        std::string vertShaderPath = m_config.vertShaderPath;
        std::string fragShaderPath = m_config.fragShaderPath;

        std::vector bindings = {
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
        };

        m_descriptorSet = std::make_unique<DescriptorSet>(
            m_context->device(),
            2,
            bindings
            );

        std::vector setLayouts = { m_descriptorSet->getLayout() };

        m_pipeline = std::make_unique<Pipeline>(
            m_context->device(),
            vk::Format::eR16G16B16A16Sfloat,
            vertShaderPath,
            fragShaderPath,
            setLayouts);
    }

    void VulkanRenderer::handleSwapchainResizing() {
        if (m_window->wasResized()) {
            vk::Extent2D size = m_window->getFramebufferSize();
            while (size.width == 0 || size.height == 0) {
                Window::waitEvents();
                size = m_window->getFramebufferSize();
            }
            m_context->device().waitIdle();
            m_swapchain->recreate();
            m_window->resetResizedFlag();
        }
    }


    void VulkanRenderer::setupUI() {
        m_imgui = std::make_unique<Imgui>(*m_context, *m_window);
    }



    VulkanRenderer::~VulkanRenderer() {
        // cleanup
    }

    void VulkanRenderer::run() {
        while (!m_window->shouldClose()) {
            Window::pollEvents();
            drawFrame();
        }

        m_context->device().waitIdle();
    }

    void VulkanRenderer::beginCommandBuffer(vk::CommandBuffer cmd) {
        cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    }

    void VulkanRenderer::prepareImageForRendering(vk::CommandBuffer cmd, vk::Image image) {
        utils::transitionImageLayout(cmd, image,
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal);
    }

    void VulkanRenderer::setupViewportAndScissor(vk::CommandBuffer cmd, vk::Extent2D extent) {
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmd.setViewport(0, viewport);

        vk::Rect2D scissor{{0, 0}, extent};
        cmd.setScissor(0, scissor);
    }

    void VulkanRenderer::bindDescriptorSets(vk::CommandBuffer cmd) {
        cmd.bindDescriptorSets(
           vk::PipelineBindPoint::eGraphics,
           m_pipeline->getLayout(),
           0,
           m_descriptorSet->getCurrentSet(m_frameManager->getFrameIndex()),
           nullptr
           );
    }

    void VulkanRenderer::drawGeometry(vk::CommandBuffer cmd) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->get());
        cmd.draw(3, 1, 0, 0);
    }

    void VulkanRenderer::renderUI(vk::CommandBuffer cmd) {
        m_imgui->createFrame();
        m_imgui->drawFrame(cmd);
    }

    void VulkanRenderer::endDynamicRendering(vk::CommandBuffer cmd) {
        cmd.endRendering();
    }

    void VulkanRenderer::prepareImageForPresent(vk::CommandBuffer cmd, vk::Image image) {
        utils::transitionImageLayout(cmd, image,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentWrite,
            {},
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR);
    }

    void VulkanRenderer::endCommandBuffer(vk::CommandBuffer cmd) {
        cmd.end();
    }

    void VulkanRenderer::submitAndPresent(uint32_t imageIndex) {
        m_frameManager->endFrame(
           m_context->graphicsQueue(),
           m_context->presentQueue(),
           m_swapchain->get(),
           imageIndex
       );
    }

    void VulkanRenderer::updateUniformBuffer(Buffer* uniformBuffer) {
        glm::mat4 identity = glm::mat4(1.0f);
        void *data = nullptr;
        vmaMapMemory(m_allocator->get(), uniformBuffer->allocation(), &data);
        memcpy(data, &identity, sizeof(glm::mat4));
        vmaUnmapMemory(m_allocator->get(), uniformBuffer->allocation());
        m_descriptorSet->updateUniformBuffer(m_frameManager->getFrameIndex(), *uniformBuffer);
    }

    void VulkanRenderer::beginDynamicRendering(vk::CommandBuffer cmd, vk::ImageView imageView, vk::Extent2D extent) {
        vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
        vk::RenderingAttachmentInfo colorAttachment{};
        colorAttachment.imageView = imageView;
        colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.clearValue = clearColor;

        vk::RenderingInfo renderingInfo{};
        renderingInfo.renderArea.offset = vk::Offset2D{0, 0};
        renderingInfo.renderArea.extent = extent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        cmd.beginRendering(renderingInfo);
    }

    void VulkanRenderer::drawFrame() {
        handleSwapchainResizing();

        uint32_t imageIndex;
        if (!m_frameManager->beginFrame(m_swapchain->get(), imageIndex)) {
            return; // Swapchain out-of-date
        }

        auto& currentFrame = m_frameManager->getCurrentFrame();
        uint32_t frameIdx = m_frameManager->getCurrentFrameIndex();
        vk::CommandBuffer cmd = currentFrame.commandBuffer;
        vk::Image targetImage = m_swapchain->getImages()[imageIndex];
        vk::ImageView targetImageView = m_swapchain->getImageViews()[imageIndex];
        vk::Extent2D extent = m_swapchain->getExtent();

        auto width = m_swapchain->getExtent().width;
        auto height = m_swapchain->getExtent().height;
        auto& i = m_msaaImages[frameIdx];
        auto ir = i->get();
        vk::Image msaaImage = m_msaaImages[frameIdx]->get();
        vk::ImageView msaaView = m_msaaColorViews[frameIdx];
        vk::Image resolveImage = m_resolveImages[frameIdx]->get();

        beginCommandBuffer(cmd);
        prepareImageForRendering(cmd, targetImage);
        beginDynamicRendering(cmd, msaaView, extent);
        setupViewportAndScissor(cmd, extent);
        updateUniformBuffer(currentFrame.uniformBuffer.get());
        bindDescriptorSets(cmd);
        drawGeometry(cmd);
        //renderUI(cmd);
        endDynamicRendering(cmd);

        utils::resolveMSAAImageTo(cmd, msaaImage, resolveImage, width, height);

        utils::blitToSwapchain(cmd, resolveImage, targetImage, width, height);

        prepareImageForPresent(cmd, targetImage);
        endCommandBuffer(cmd);

        submitAndPresent(imageIndex);
    }

    void VulkanRenderer::createMSAAImage() {
        vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e4;

        // set format to HDR capable space
        vk::Format format = vk::Format::eR16G16B16A16Sfloat;
;
        constexpr vk::ImageUsageFlags usage =
            vk::ImageUsageFlagBits::eColorAttachment |
            vk::ImageUsageFlagBits::eInputAttachment |
            vk::ImageUsageFlagBits::eTransferSrc;

        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = m_swapchain->getExtent().width;
        imageInfo.extent.height = m_swapchain->getExtent().height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = usage;
        imageInfo.samples = msaaSamples;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        size_t framesInFlight = m_frameManager->getFramesInFlightCount();

        m_msaaImages.resize(framesInFlight);
        m_msaaColorViews.resize(framesInFlight);

        for (size_t i = 0; i < framesInFlight; ++i) {
            // Create the Image object
            m_msaaImages[i] = std::make_unique<Image>(*m_allocator, imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = m_msaaImages[i]->get(); // Get the VkImage from your new Image object
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            m_msaaColorViews[i] = m_context->device().createImageView(viewInfo);
        }
    }

    void VulkanRenderer::createResolveImages() {
        vk::Format format = vk::Format::eR16G16B16A16Sfloat; // Match your swapchain/attachment format
        vk::Extent2D extent = m_swapchain->getExtent();

        size_t framesInFlight = m_frameManager->getFramesInFlightCount();
        m_resolveImages.resize(framesInFlight);
        m_resolveViews.resize(framesInFlight);

        for (size_t i = 0; i < framesInFlight; ++i) {
            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D;
            imageInfo.extent.width = extent.width;
            imageInfo.extent.height = extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = vk::ImageTiling::eOptimal;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;
            imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
            imageInfo.samples = vk::SampleCountFlagBits::e1; // Resolve is NOT multisampled!
            imageInfo.sharingMode = vk::SharingMode::eExclusive;

            // Create the Image (assume you have your own Image wrapper e.g. using VMA)
            m_resolveImages[i] = std::make_unique<Image>(*m_allocator, imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = m_resolveImages[i]->get();
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            m_resolveViews[i] = m_context->device().createImageView(viewInfo);
        }

    }



}
