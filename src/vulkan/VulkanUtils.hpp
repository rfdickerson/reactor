#pragma once

#include <vulkan/vulkan.hpp>

namespace reactor::utils
{

inline void setupViewportAndScissor(vk::CommandBuffer cmd, vk::Extent2D extent)
{
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmd.setViewport(0, viewport);

    const vk::Rect2D scissor{{0, 0}, extent};
    cmd.setScissor(0, scissor);
}

inline vk::SampleCountFlagBits mapSampleCountFlag(uint32_t sampleCount)
{
    switch (sampleCount)
    {
    case 1:
        return vk::SampleCountFlagBits::e1;
    case 2:
        return vk::SampleCountFlagBits::e2;
    case 4:
        return vk::SampleCountFlagBits::e4;
    case 8:
        return vk::SampleCountFlagBits::e8;
    default:
        return vk::SampleCountFlagBits::e1;
    }
}

} // namespace reactor::utils