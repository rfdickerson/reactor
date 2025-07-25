//
// Created by rfdic on 6/13/2025.
//

#include "Allocator.hpp"

#include <spdlog/spdlog.h>

#include "Buffer.hpp"

namespace reactor
{

Allocator::Allocator(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance, vk::Queue graphicsQueue, uint32_t graphicsQueueFamilyIndex)
    : m_device(device), m_graphicsQueue(graphicsQueue), m_graphicQueueFamilyIndex(graphicsQueueFamilyIndex)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    vmaCreateAllocator(&allocatorInfo, &m_allocator);

    spdlog::info("Allocator created");
}

Allocator::~Allocator()
{
    if (m_allocator)
        vmaDestroyAllocator(m_allocator);
}

std::unique_ptr<Buffer> Allocator::createBufferWithData(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage, const std::string& name)
{
    // Create CPU-visible staging buffer
    Buffer stagingBuffer(
        *this,
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_ONLY,
        name + " Staging");

    // Map and copy data to staging buffer
    void* mappedData;
    vmaMapMemory(m_allocator, stagingBuffer.allocation(), &mappedData);
    memcpy(mappedData, data, size);
    vmaUnmapMemory(m_allocator, stagingBuffer.allocation());

    // Create GPU-local destination buffer
    // Add the transfer destination usage flag
    usage |= vk::BufferUsageFlagBits::eTransferDst;
    auto destBuffer = std::make_unique<Buffer>(
        *this,
        size,
        usage,
        VMA_MEMORY_USAGE_GPU_ONLY,
        name);

    // Perform the copy
    immediateSubmit([&](vk::CommandBuffer cmd) {
        vk::BufferCopy copyRegion(0, 0, size);
        cmd.copyBuffer(stagingBuffer.getHandle(), destBuffer->getHandle(), 1, &copyRegion);
    });

    return destBuffer;
}

void Allocator::immediateSubmit(
    std::function<void(vk::CommandBuffer cmd)>&& function)
{
    // Create a temporary command pool and buffer
    vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eTransient, m_graphicQueueFamilyIndex);
    vk::CommandPool cmdPool = m_device.createCommandPool(poolInfo);

    vk::CommandBufferAllocateInfo allocInfo(cmdPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer cmd = m_device.allocateCommandBuffers(allocInfo)[0];

    // Record and submit commands
    cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    function(cmd); // Execute the provided command recording lambda
    cmd.end();

    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &cmd);
    m_graphicsQueue.submit(submitInfo, nullptr);
    m_graphicsQueue.waitIdle(); // Wait for the operation to complete

    // Clean up temporary resources
    m_device.freeCommandBuffers(cmdPool, cmd);
    m_device.destroyCommandPool(cmdPool);
}

} // namespace reactor