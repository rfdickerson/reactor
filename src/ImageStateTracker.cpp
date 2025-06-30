//
// Created by rfdic on 6/29/2025.
//

#include "ImageStateTracker.h"
#include <stdexcept>

namespace reactor {

void ImageStateTracker::recordState(vk::Image image, vk::ImageLayout initialState) {
    m_imageStates[image] = initialState;
}

vk::ImageLayout ImageStateTracker::getCurrentLayout(vk::Image image) const {
    auto it = m_imageStates.find(image);
    if (it == m_imageStates.end()) {
        // Or you could default to eUndefined and record it, depending on desired strictness
        throw std::runtime_error("Attempted to get layout of an untracked image.");
    }
    return it->second;
}

void ImageStateTracker::transition(
    vk::CommandBuffer cmd,
    vk::Image image,
    vk::ImageLayout newLayout,
    vk::PipelineStageFlags srcStage,
    vk::PipelineStageFlags dstStage,
    vk::AccessFlags srcAccess,
    vk::AccessFlags dstAccess,
    vk::ImageAspectFlags aspectFlags
) {
    const vk::ImageLayout oldLayout = getCurrentLayout(image);

    // If the layout is already correct, we don't need a barrier.
    if (oldLayout == newLayout) {
        return;
    }

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    cmd.pipelineBarrier(
        srcStage,       // Stage where operations occurred before the barrier
        dstStage,       // Stage where operations will wait for the barrier
        {},             // Dependency flags
        nullptr,        // Memory barriers
        nullptr,        // Buffer memory barriers
        barrier         // Image memory barriers
    );

    // IMPORTANT: Update the internal state to the new layout.
    m_imageStates[image] = newLayout;
}


} // reactor