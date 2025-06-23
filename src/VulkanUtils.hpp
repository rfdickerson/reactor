#pragma once

#include <vulkan/vulkan.hpp>

namespace reactor::utils {

    /**
     * @brief Records a pipeline barrier to transition the layout of a Vulkan image.
     *
     * This is a general-purpose utility that provides full control over the
     * synchronization parameters of the barrier.
     *
     * @param cmd The command buffer to record the barrier into.
     * @param image The image to transition.
     * @param srcStageMask The pipeline stage(s) that must complete before the barrier.
     * @param dstStageMask The pipeline stage(s) that must wait for the barrier.
     * @param srcAccessMask The access types that must complete before the barrier.
     * @param dstAccessMask The access types that will be enabled after the barrier.
     * @param oldLayout The current layout of the image.
     * @param newLayout The desired new layout for the image.
     */
    inline void transitionImageLayout(
        vk::CommandBuffer cmd,
        vk::Image image,
        vk::PipelineStageFlags srcStageMask,
        vk::PipelineStageFlags dstStageMask,
        vk::AccessFlags srcAccessMask,
        vk::AccessFlags dstAccessMask,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout) {

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;

        cmd.pipelineBarrier(
            srcStageMask,
            dstStageMask,
            {},
            nullptr, nullptr, barrier
        );
    }

    void blitToSwapchain(
        vk::CommandBuffer cmd,
        vk::Image msaaImage,
        vk::Image swapchainImage,
        uint32_t width,
        uint32_t height
        ) {

        transitionImageLayout(
            cmd,
            msaaImage,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eTransfer,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::AccessFlagBits::eTransferRead,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::eTransferSrcOptimal);

        // transition the blit
        transitionImageLayout(
            cmd,
            swapchainImage,
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            vk::AccessFlags(0),
            vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal
            );

        // Define the blit region (full image)
        vk::ImageBlit blitRegion{};
        blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.srcSubresource.mipLevel = 0;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcOffsets[0] = vk::Offset3D{0, 0, 0};
        blitRegion.srcOffsets[1] = vk::Offset3D{
            static_cast<int32_t>(width),
            static_cast<int32_t>(height),
            1
        };

        blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.dstSubresource.mipLevel = 0;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstOffsets[0] = vk::Offset3D{0, 0, 0};
        blitRegion.dstOffsets[1] = vk::Offset3D{
            static_cast<int32_t>(width),
            static_cast<int32_t>(height),
            1
        };

        // Blit the MSAA image to the swapchain image, converting formats
        cmd.blitImage(
            msaaImage, vk::ImageLayout::eTransferSrcOptimal,
            swapchainImage, vk::ImageLayout::eTransferDstOptimal,
            1, &blitRegion,
            vk::Filter::eNearest // Use vk::Filter::eLinear if filtering is desired
        );

        // Transition the swapchain image for presentation
        transitionImageLayout(
            cmd,
            swapchainImage,
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::AccessFlagBits::eColorAttachmentWrite,
            {},
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::ePresentSrcKHR);

    }

    inline void resolveMSAAImageTo(
        const vk::CommandBuffer cmd,
        const vk::Image msaaImage,
        const vk::Image resolveImage,
        const uint32_t width,
        const uint32_t height) {
        vk::ImageResolve resolveRegion{};
        resolveRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        resolveRegion.srcSubresource.mipLevel = 0;
        resolveRegion.srcSubresource.baseArrayLayer = 0;
        resolveRegion.srcSubresource.layerCount = 1;
        resolveRegion.srcOffset = vk::Offset3D{0, 0, 0};
        resolveRegion.dstSubresource = resolveRegion.srcSubresource;
        resolveRegion.dstOffset = vk::Offset3D{0, 0, 0};
        resolveRegion.extent = vk::Extent3D{width, height, 1};

        cmd.resolveImage(
            msaaImage,      // srcImage
            vk::ImageLayout::eTransferSrcOptimal,
            resolveImage,   // dstImage
            vk::ImageLayout::eTransferDstOptimal,
            1, &resolveRegion
        );

    }

} // namespace reactor::utils