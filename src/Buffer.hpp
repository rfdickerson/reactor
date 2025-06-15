#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "Allocator.hpp"

namespace reactor {

class Buffer {
public:
    Buffer(Allocator& allocator, vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage);

    ~Buffer();

    // Non-copyable
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    [[nodiscard]] vk::Buffer buffer() const { return m_buffer; }
    [[nodiscard]] VmaAllocation allocation() const { return m_allocation; }
    [[nodiscard]] vk::DeviceSize size() const { return m_size; }



private:
    Allocator& m_allocator;
    vk::Buffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    vk::DeviceSize m_size = 0;
};

} // reactor

