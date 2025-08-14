//
// Created by rfdic on 6/29/2025.
//

#include "UniformManager.hpp"

namespace reactor
{

UniformManager::UniformManager(Allocator& allocator, size_t framesInFlight)
    : m_allocator(allocator), m_framesInFlightCount(framesInFlight)
{}

std::vector<std::unique_ptr<Buffer>> UniformManager::createFrameSpecificBuffers(vk::DeviceSize size)
{
    std::vector<std::unique_ptr<Buffer>> buffers;
    buffers.reserve(m_framesInFlightCount);

    for (size_t frame = 0; frame < m_framesInFlightCount; ++frame)
    {
        buffers.emplace_back(std::make_unique<Buffer>(
            m_allocator, size, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU, "Uniform buffer"));
    }

    return buffers;
}

} // namespace reactor