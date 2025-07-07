//
// Created by rfdic on 6/29/2025.
//

#include "UniformManager.hpp"

namespace reactor {

 UniformManager::UniformManager(Allocator &allocator, size_t framesInFlight)
     : m_allocator(allocator), m_framesInFlight(framesInFlight) {  }

std::vector<std::unique_ptr<Buffer>>
 UniformManager::createFrameSpecificBuffers(vk::DeviceSize size) {
     std::vector<std::unique_ptr<Buffer>> buffers;
     for (size_t i = 0; i < m_framesInFlight; i++) {
         buffers.push_back(std::make_unique<Buffer>(
             m_allocator,
             size,
             vk::BufferUsageFlagBits::eUniformBuffer,
             VMA_MEMORY_USAGE_CPU_TO_GPU, "Uniform buffer"));
     }
     return buffers;
 }


} // reactor