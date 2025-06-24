#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "Allocator.hpp"

namespace reactor {

    class Image {
    public:
        Image(Allocator& allocator,
            const vk::ImageCreateInfo& imageInfo,
            VmaMemoryUsage memoryUsage);

        ~Image();

        // Prevent copying and assignment
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        // Allow moving
        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;

        vk::Image get() const { return m_image; }

    private:
        Allocator& m_allocator;
        vk::Image m_image = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        void cleanup();
    };
}