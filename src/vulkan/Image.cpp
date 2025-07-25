//
// Created by rfdic on 6/17/2025.
//

#include "Image.hpp"

namespace reactor {

    Image::Image(Allocator &allocator, const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage)
        : m_allocator(allocator) {
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memoryUsage;

        const VkResult result = vmaCreateImage(
            m_allocator.getAllocator(),
            reinterpret_cast<const VkImageCreateInfo*>(&imageInfo),
            &allocInfo,
            reinterpret_cast<VkImage*>(&m_image),
            &m_allocation,
            nullptr);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }
    }

    Image::~Image() {
        cleanup();
    }

    Image::Image(Image&& other) noexcept
        : m_allocator(other.m_allocator), // Initialize reference in move constructor
          m_image(other.m_image),
          m_allocation(other.m_allocation)
    {
        other.m_image = VK_NULL_HANDLE;
        other.m_allocation = VK_NULL_HANDLE;
    }

    Image& Image::operator=(Image&& other) noexcept {
        if (this != &other) {
            // First, clean up the resources currently held by *this* object.
            cleanup();

            // Transfer ownership of resources from 'other' to 'this'.
            m_image = other.m_image;
            m_allocation = other.m_allocation;

            // Invalidate 'other' to prevent it from cleaning up the moved resources.
            other.m_image = VK_NULL_HANDLE;
            other.m_allocation = VK_NULL_HANDLE;

            // **DO NOT** attempt to assign to m_allocator.
            // m_allocator is a reference and must remain bound to its original Allocator instance.
            // It was already initialized in the constructor and cannot be changed.
        }
        return *this;
    }

    void Image::cleanup() {
        if (m_image && m_allocation) {
            vmaDestroyImage(m_allocator.getAllocator(), m_image, m_allocation);
            spdlog::info("Image destroyed");
            m_image = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }
    }

} // reactor