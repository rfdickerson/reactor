#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace reactor {

class Allocator {
public:
    Allocator(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance);
    ~Allocator();

    VmaAllocator get() const { return m_allocator; }

    // Non-copyable
    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;

};

} // reactor


