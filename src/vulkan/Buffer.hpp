#pragma once

#include "Allocator.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace reactor {

class Buffer {
public:
    Buffer(Allocator& allocator, vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, std::string name="");

    ~Buffer();

    [[nodiscard]] const std::string& getName() const { return m_name; }

    // Non-copyable
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    [[nodiscard]] vk::Buffer getHandle() const { return m_buffer; }
    [[nodiscard]] VmaAllocation allocation() const { return m_allocation; }
    [[nodiscard]] vk::DeviceSize size() const { return m_size; }



private:
    Allocator& m_allocator;
    vk::Buffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    vk::DeviceSize m_size = 0;

    std::string m_name;
};

} // reactor

