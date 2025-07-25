#pragma once

#include "Buffer.hpp"
#include "Vertex.hpp"
#include <vector>

#include <glm/glm.hpp>

namespace reactor
{

class Mesh
{
public:
    Mesh(Allocator& allocator, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    // Movable but not copyable (due to Buffer)
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    ~Mesh() = default;

    vk::Buffer getVertexBuffer() const
    {
        return m_vertexBuffer->getHandle();
    }
    vk::Buffer getIndexBuffer() const
    {
        return m_indexBuffer->getHandle();
    }
    uint32_t getIndexCount() const
    {
        return m_indexCount;
    }

private:
    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;
    uint32_t m_indexCount = 0;

    void createBuffer(Allocator& allocator, const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage, std::unique_ptr<Buffer>& buffer, const std::string& name);
};

} // namespace reactor
