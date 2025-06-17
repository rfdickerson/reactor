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

} // namespace reactor::utils