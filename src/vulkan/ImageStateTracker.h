#pragma once

#include <vulkan/vulkan.hpp>
#include <map>

namespace reactor {

class ImageStateTracker {
public:
    ImageStateTracker() = default;

    void recordState(vk::Image image, vk::ImageLayout initialState = vk::ImageLayout::eUndefined);

    void transition(
        vk::CommandBuffer cmd,
        vk::Image image,
        vk::ImageLayout newLayout,
        vk::PipelineStageFlags srcStage,
        vk::PipelineStageFlags dstStage,
        vk::AccessFlags srcAccess,
        vk::AccessFlags dstAccess,
        vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);

    vk::ImageLayout getCurrentLayout(vk::Image image) const;

private:
    std::map<vk::Image, vk::ImageLayout> m_imageStates;
};

} // namespace reactor

