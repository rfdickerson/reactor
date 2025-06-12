#include "VulkanRenderer.hpp"

#include "Window.hpp"

namespace reactor {
    VulkanRenderer::VulkanRenderer() {
        // create the window first
        m_window = std::make_unique<Window>(1280, 720, "Reactor");
        m_context = std::make_unique<VulkanContext>(m_window->getNativeWindow());
        m_swapchain = std::make_unique<Swapchain>(
            m_context->device(),
            m_context->physicalDevice(),
            m_context->surface(), *m_window);

        m_frameManager = std::make_unique<FrameManager>(m_context->device(), 0, 2, m_swapchain->getImageViews().size());

        std::string vertShaderPath = "../shaders/triangle.vert.spv";
        std::string fragShaderPath = "../shaders/triangle.frag.spv";


        m_pipeline = std::make_unique<Pipeline>(
            m_context->device(),
            vk::Format::eB8G8R8A8Srgb,
            vertShaderPath,
            fragShaderPath);
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


    void VulkanRenderer::drawFrame() {
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

        uint32_t imageIndex = 0;
        if (!m_frameManager->beginFrame(m_swapchain->get(), imageIndex)) {
            // Swapchain out-of-date, try next frame
            return;
        }

        auto& frame = m_frameManager->getCurrentFrame();
        auto cmd = frame.commandBuffer;

        // Setup clear color
        vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
        vk::Extent2D extent = m_swapchain->getExtent();

        // Get the correct swapchain image view for the acquired image
        const auto& imageViews = m_swapchain->getImageViews();
        vk::ImageView imageView = imageViews[imageIndex];

        // Setup rendering info for dynamic rendering
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

        cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

        // Before dynamic rendering, ensure correct image layout for color attachment
        {
            vk::ImageMemoryBarrier imageBarrier{};
            imageBarrier.oldLayout = vk::ImageLayout::eUndefined;  // see note below
            imageBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
            imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.image = m_swapchain->getImages()[imageIndex];
            imageBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageBarrier.subresourceRange.baseMipLevel = 0;
            imageBarrier.subresourceRange.levelCount = 1;
            imageBarrier.subresourceRange.baseArrayLayer = 0;
            imageBarrier.subresourceRange.layerCount = 1;
            imageBarrier.srcAccessMask = {};
            imageBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

            cmd.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {},
                nullptr, nullptr, imageBarrier
            );
        }


        // Begin dynamic rendering
        cmd.beginRendering(renderingInfo);

        // Set dynamic viewport/scissor
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

        // Bind pipeline and draw triangle
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->get());
        cmd.draw(3, 1, 0, 0);

        cmd.endRendering();

        // Transition swapchain image (the one we're presenting) to PRESENT_SRC_KHR
        {
            vk::ImageMemoryBarrier barrier{};
            barrier
                .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setImage(m_swapchain->getImages()[imageIndex])
                .setSubresourceRange(
                    vk::ImageSubresourceRange()
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setBaseMipLevel(0)
                        .setLevelCount(1)
                        .setBaseArrayLayer(0)
                        .setLayerCount(1))
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask({}); // No need to access in present

            cmd.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {},
                nullptr, nullptr, barrier
            );
        }


        cmd.end();

        // Submit and present
        m_frameManager->endFrame(
            m_context->graphicsQueue(),
            m_context->presentQueue(),
            m_swapchain->get(),
            imageIndex
        );
    }

}
