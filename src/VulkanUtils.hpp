#pragma once

#include <vulkan/vulkan.hpp>

namespace reactor::utils {



inline void resolveMSAAImageTo(vk::CommandBuffer cmd, vk::Image msaaImage, vk::Image resolveImage,
                               uint32_t width, uint32_t height) {



    vk::ImageResolve resolveRegion{};
    resolveRegion.srcSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    resolveRegion.srcSubresource.mipLevel       = 0;
    resolveRegion.srcSubresource.baseArrayLayer = 0;
    resolveRegion.srcSubresource.layerCount     = 1;
    resolveRegion.srcOffset                     = vk::Offset3D{0, 0, 0};
    resolveRegion.dstSubresource                = resolveRegion.srcSubresource;
    resolveRegion.dstOffset                     = vk::Offset3D{0, 0, 0};
    resolveRegion.extent                        = vk::Extent3D{width, height, 1};

    cmd.resolveImage(msaaImage, // srcImage
                     vk::ImageLayout::eTransferSrcOptimal,
                     resolveImage, // dstImage
                     vk::ImageLayout::eTransferDstOptimal, 1, &resolveRegion);
}

inline void setupViewportAndScissor(vk::CommandBuffer cmd, vk::Extent2D extent) {
    vk::Viewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(extent.width);
    viewport.height   = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmd.setViewport(0, viewport);

    const vk::Rect2D scissor{{0, 0}, extent};
    cmd.setScissor(0, scissor);
}

} // namespace reactor::utils