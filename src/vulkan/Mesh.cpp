#include "Mesh.hpp"
#include <cstring>  // For memcpy

#include "Allocator.hpp"

namespace reactor {

// Forward declaration of a helper function to perform the copy.
// This keeps the constructor clean.
// Helper function for one-time command submission.
// This could be placed in a utility file or within the Allocator class.
void immediateSubmit(
    Allocator& allocator,
    std::function<void(vk::CommandBuffer cmd)>&& function)
{
    vk::Device device = allocator.getDevice();
    vk::Queue graphicsQueue = allocator.getGraphicsQueue();
    uint32_t queueFamilyIndex = allocator.getGraphicsQueueFamilyIndex();

    // Create a temporary command pool and buffer
    vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eTransient, queueFamilyIndex);
    vk::CommandPool cmdPool = device.createCommandPool(poolInfo);

    vk::CommandBufferAllocateInfo allocInfo(cmdPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer cmd = device.allocateCommandBuffers(allocInfo)[0];

    // Record and submit commands
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd.begin(beginInfo);

    function(cmd); // Execute the provided command recording function

    cmd.end();

    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &cmd);
    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle(); // Wait for the operation to complete

    // Clean up temporary resources
    device.freeCommandBuffers(cmdPool, cmd);
    device.destroyCommandPool(cmdPool);
}

Mesh::Mesh(Allocator& allocator, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    vk::DeviceSize vertexSize = vertices.size() * sizeof(Vertex);
    vk::DeviceSize indexSize = indices.size() * sizeof(uint32_t);
    m_indexCount = static_cast<uint32_t>(indices.size());

    // Staging buffers (CPU-visible)
    Buffer stagingVertex(allocator, vertexSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_TO_GPU, "Staging Vertex");
    void* data;
    vmaMapMemory(allocator.getAllocator(), stagingVertex.allocation(), &data);
    memcpy(data, vertices.data(), static_cast<size_t>(vertexSize));
    vmaUnmapMemory(allocator.getAllocator(), stagingVertex.allocation());

    Buffer stagingIndex(allocator, indexSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_TO_GPU, "Staging Index");
    vmaMapMemory(allocator.getAllocator(), stagingIndex.allocation(), &data);
    memcpy(data, indices.data(), static_cast<size_t>(indexSize));
    vmaUnmapMemory(allocator.getAllocator(), stagingIndex.allocation());

    // GPU buffers (device-local)
    m_vertexBuffer = std::make_unique<Buffer>(allocator, vertexSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_GPU_ONLY, "Vertex Buffer");
    m_indexBuffer = std::make_unique<Buffer>(allocator, indexSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, VMA_MEMORY_USAGE_GPU_ONLY, "Index Buffer");


    // One-time transfer from staging to device-local buffers
    immediateSubmit(allocator, [&](vk::CommandBuffer cmd) {
        vk::BufferCopy vertexCopyRegion{};
        vertexCopyRegion.srcOffset = 0;
        vertexCopyRegion.dstOffset = 0;
        vertexCopyRegion.size = vertexSize;
        cmd.copyBuffer(stagingVertex.getHandle(), m_vertexBuffer->getHandle(), 1, &vertexCopyRegion);

        vk::BufferCopy indexCopyRegion{};
        indexCopyRegion.srcOffset = 0;
        indexCopyRegion.dstOffset = 0;
        indexCopyRegion.size = indexSize;
        cmd.copyBuffer(stagingIndex.getHandle(), m_indexBuffer->getHandle(), 1, &indexCopyRegion);
    });


}


Mesh::Mesh(Mesh&& other) noexcept
    : m_vertexBuffer(std::move(other.m_vertexBuffer)),
      m_indexBuffer(std::move(other.m_indexBuffer)),
      m_indexCount(other.m_indexCount) {
    other.m_indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        m_vertexBuffer = std::move(other.m_vertexBuffer);
        m_indexBuffer = std::move(other.m_indexBuffer);
        m_indexCount = other.m_indexCount;
        other.m_indexCount = 0;
    }
    return *this;
}

} // namespace reactor