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
            vk::Format::eB8G8R8A8Srgb,
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
        vk::CommandBuffer cmd = currentFrame.commandBuffer;
        vk::Image targetImage = m_swapchain->getImages()[imageIndex];
        vk::ImageView targetImageView = m_swapchain->getImageViews()[imageIndex];
        vk::Extent2D extent = m_swapchain->getExtent();

        beginCommandBuffer(cmd);
        prepareImageForRendering(cmd, targetImage);
        beginDynamicRendering(cmd, targetImageView, extent);
        setupViewportAndScissor(cmd, extent);
        updateUniformBuffer(currentFrame.uniformBuffer.get());
        bindDescriptorSets(cmd);
        drawGeometry(cmd);
        renderUI(cmd);
        endDynamicRendering(cmd);
        prepareImageForPresent(cmd, targetImage);
        endCommandBuffer(cmd);

        submitAndPresent(imageIndex);
    }

}
