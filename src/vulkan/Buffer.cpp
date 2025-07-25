//
// Created by rfdic on 6/14/2025.
//

#include "Buffer.hpp"

#include <spdlog/spdlog.h>

namespace reactor {

    Buffer::Buffer(Allocator &allocator, vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, std::string name)
        : m_allocator(allocator), m_size(size), m_name(name)
    {
        vk::BufferCreateInfo bufferInfo = {};
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memoryUsage;

        const VkResult result = vmaCreateBuffer(
            m_allocator.getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo *>(&bufferInfo),
            &allocInfo,
            reinterpret_cast<VkBuffer *>(&m_buffer),
            &m_allocation,
            nullptr);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }
    }

    Buffer::~Buffer() {
        if (m_buffer && m_allocation) {
            vmaDestroyBuffer(m_allocator.getAllocator(), m_buffer, m_allocation);

            spdlog::info("Buffer {} destroyed", m_name.c_str());

            m_buffer = nullptr;
            m_allocation = nullptr;
        }
    }


} // reactor