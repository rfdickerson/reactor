#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

#include "Image.hpp"

namespace reactor::utils
{
class ImageBuilder
{
public:
    ImageBuilder(const vk::Device& device, Allocator& allocator, const vk::Extent2D& defaultExtent);

    ImageBuilder& setFormat(vk::Format);
    ImageBuilder& setUsage(vk::ImageUsageFlags usage);
    ImageBuilder& setSamples(vk::SampleCountFlagBits samples);
    ImageBuilder& setAspectMask(vk::ImageAspectFlags aspect);

    struct BuiltImage
    {
        std::unique_ptr<Image> image;
        vk::ImageView view;
    };

    BuiltImage build();

private:
    const vk::Device& m_device;
    Allocator& m_allocator;
    vk::Extent2D m_extent;

    vk::ImageCreateInfo m_imageInfo{};
    vk::ImageViewCreateInfo m_viewInfo{};
};
} // namespace reactor::utils