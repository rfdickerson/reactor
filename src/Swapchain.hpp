//
// Created by rfdic on 6/9/2025.
//

#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include <vector>
#include <vulkan/vulkan.hpp>

#include "Window.hpp"

namespace reactor {
    class Swapchain {
    public:
        Swapchain(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Window& window);
        ~Swapchain();

        // Public interface
        [[nodiscard]] vk::SwapchainKHR get() const { return m_swapchain; }
        [[nodiscard]] vk::Format getFormat() const { return m_format; }
        [[nodiscard]] vk::Extent2D getExtent() const { return m_extent; }
        [[nodiscard]] const std::vector<vk::ImageView>& getImageViews() const { return m_imageViews; }

        void recreate();

    private:
        void cleanup();

        vk::Device m_device; // Store a copy for cleanup
        vk::SwapchainKHR m_swapchain;
        std::vector<vk::Image> m_images;
        std::vector<vk::ImageView> m_imageViews;
        vk::Format m_format;
        vk::Extent2D m_extent;

        vk::PhysicalDevice m_physicalDevice;
        vk::SurfaceKHR m_surface;
        const Window& m_window; // non-owning
    };
}

#endif //SWAPCHAIN_HPP
