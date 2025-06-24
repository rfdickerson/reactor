#include "Swapchain.hpp"

namespace reactor {

    // Helper structures and selection for formats/present modes
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for (const auto& format : availableFormats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }
        return availableFormats[0];
    }

    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for (const auto& presentMode : availablePresentModes) {
            if (presentMode == vk::PresentModeKHR::eMailbox) {
                return presentMode;
            }
        }
        return vk::PresentModeKHR::eFifo; // guaranteed to be available
    }

    static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            vk::Extent2D actualExtent = window.getFramebufferSize();
            actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    Swapchain::Swapchain(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Window& window)
        : m_device(device), m_physicalDevice(physicalDevice), m_surface(surface), m_window(window) {

        // Query surface capabilities
        auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        auto formats      = physicalDevice.getSurfaceFormatsKHR(surface);
        auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
        vk::Extent2D extent = chooseSwapExtent(capabilities, window);

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo = {};
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

        // Assume graphics and present queue families are the same for simplicity
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        m_swapchain = m_device.createSwapchainKHR(createInfo);
        m_images = m_device.getSwapchainImagesKHR(m_swapchain);
        m_format = surfaceFormat.format;
        m_extent = extent;

        // Create image views
        m_imageViews.reserve(m_images.size());
        for (auto image : m_images) {
            vk::ImageViewCreateInfo viewInfo = {};
            viewInfo.image = image;
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = m_format;
            viewInfo.components = {
                vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity
            };
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            m_imageViews.push_back(m_device.createImageView(viewInfo));
        }

        spdlog::info("Created swapchain: format = {}, extent = {}x{}, images = {}, present mode = {}",
        static_cast<int>(surfaceFormat.format),
        extent.width, extent.height,
        m_images.size(),
        static_cast<int>(presentMode)
    );

    }

    Swapchain::~Swapchain() {
        cleanup();
    }

    void Swapchain::cleanup() {
        spdlog::info("Cleaning up swapchain resources");

        for (auto view : m_imageViews) {
            m_device.destroyImageView(view);
        }
        if (m_swapchain) {
            m_device.destroySwapchainKHR(m_swapchain);
            m_swapchain = nullptr;
        }
        m_imageViews.clear();
        m_images.clear();
    }

    void Swapchain::recreate() {
        spdlog::info("Recreating swapchain...");

        cleanup();

        // Query and (re-)create swapchain just like constructor
        auto capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
        auto formats      = m_physicalDevice.getSurfaceFormatsKHR(m_surface);
        auto presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
        vk::Extent2D extent = chooseSwapExtent(capabilities, m_window);

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo = {};
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        m_swapchain = m_device.createSwapchainKHR(createInfo);
        m_images = m_device.getSwapchainImagesKHR(m_swapchain);
        m_format = surfaceFormat.format;
        m_extent = extent;

        m_imageViews.reserve(m_images.size());
        for (auto image : m_images) {
            vk::ImageViewCreateInfo viewInfo = {};
            viewInfo.image = image;
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = m_format;
            viewInfo.components = {
                vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity
            };
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            m_imageViews.push_back(m_device.createImageView(viewInfo));
        }

        spdlog::info("Swapchain recreated: format = {}, extent = {}x{}, images = {}, present mode = {}",
        static_cast<int>(m_format),
        m_extent.width, m_extent.height,
        m_images.size(),
        static_cast<int>(chooseSwapPresentMode(
            m_physicalDevice.getSurfacePresentModesKHR(m_surface)))
    );

    }

}
