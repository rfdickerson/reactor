#include "ImageUtils.hpp"

namespace reactor::utils
{

ImageBuilder::ImageBuilder(const vk::Device& device, Allocator& allocator, const vk::Extent2D& defaultExtent)
    : m_device(device), m_allocator(allocator), m_extent(defaultExtent)
{
    m_imageInfo.imageType = vk::ImageType::e2D;
    m_imageInfo.extent = vk::Extent3D{m_extent.width, m_extent.height, 1};
    m_imageInfo.mipLevels = 1;
    m_imageInfo.arrayLayers = 1;
    m_imageInfo.tiling = vk::ImageTiling::eOptimal;
    m_imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    m_imageInfo.sharingMode = vk::SharingMode::eExclusive;
    m_imageInfo.samples = vk::SampleCountFlagBits::e1;

    m_viewInfo.viewType = vk::ImageViewType::e2D;
    m_viewInfo.subresourceRange.baseMipLevel = 0;
    m_viewInfo.subresourceRange.levelCount = 1;
    m_viewInfo.subresourceRange.baseArrayLayer = 0;
    m_viewInfo.subresourceRange.layerCount = 1;
    m_viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
}

ImageBuilder& ImageBuilder::setFormat(vk::Format format)
{
    m_imageInfo.format = format;
    m_viewInfo.format = format;
    return *this;
}

ImageBuilder& ImageBuilder::setUsage(vk::ImageUsageFlags usage)
{
    m_imageInfo.usage = usage;
    return *this;
}

ImageBuilder& ImageBuilder::setSamples(vk::SampleCountFlagBits samples)
{
    m_imageInfo.samples = samples;
    return *this;
}

ImageBuilder& ImageBuilder::setAspectMask(vk::ImageAspectFlags aspect)
{
    m_viewInfo.subresourceRange.aspectMask = aspect;
    return *this;
}

ImageBuilder::BuiltImage ImageBuilder::build()
{
    auto image = std::make_unique<Image>(m_allocator, m_imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

    m_viewInfo.image = image->get();
    vk::ImageView view = m_device.createImageView(m_viewInfo);

    return {.image = std::move(image), .view = view};
}

} // namespace reactor::utils