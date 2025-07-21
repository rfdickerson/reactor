#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace reactor
{
class Buffer;

class Allocator
{
public:
    Allocator(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance, vk::Queue graphicsQueue, uint32_t graphicsQueueFamilyIndex);
    ~Allocator();

    VmaAllocator getAllocator() const
    {
        return m_allocator;
    }
    vk::Device getDevice() const
    {
        return m_device;
    }
    vk::Queue getGraphicsQueue() const
    {
        return m_graphicsQueue;
    }
    uint32_t getGraphicsQueueFamilyIndex() const
    {
        return m_graphicQueueFamilyIndex;
    }

    // New factory method
    std::unique_ptr<Buffer> createBufferWithData(
        const void* data,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        const std::string& name = "");

    // Non-copyable
    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

private:

    void immediateSubmit(std::function<void(vk::CommandBuffer cmd)>&& function);

    VmaAllocator m_allocator = nullptr;
    vk::Device m_device;
    vk::Queue m_graphicsQueue;
    uint32_t m_graphicQueueFamilyIndex;
};

} // namespace reactor
