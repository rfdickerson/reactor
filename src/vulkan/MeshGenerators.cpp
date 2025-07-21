#include "MeshGenerators.hpp"
#include "Vertex.hpp"

namespace reactor
{
std::vector<Vertex> generatePlaneVertices(int subdivisions, float size)
{
    std::vector<Vertex> vertices;
    int rows = subdivisions + 1;
    vertices.reserve(rows * rows);
    for (int z = 0; z < rows; ++z)
    {
        for (int x = 0; x < rows; ++x)
        {
            float u = static_cast<float>(x) / subdivisions;
            float v = static_cast<float>(z) / subdivisions;
            Vertex vert{};
            vert.pos = glm::vec3((u - 0.5f) * size, 0.0f, (v - 0.5f) * size);
            vert.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vert.color = glm::vec3(0.2f, 0.8f, 0.2f); // Greenish ground
            vert.texCoord = glm::vec2(u, v);
            vertices.push_back(vert);
        }
    }
    return vertices;
}

std::vector<uint32_t> generatePlaneIndices(int subdivisions)
{
    std::vector<uint32_t> indices;
    int rows = subdivisions + 1;
    for (int z = 0; z < subdivisions; ++z)
    {
        for (int x = 0; x < subdivisions; ++x)
        {
            uint32_t bottomLeft = z * rows + x;
            uint32_t bottomRight = bottomLeft + 1;
            uint32_t topLeft = bottomLeft + rows;
            uint32_t topRight = topLeft + 1;
            indices.insert(indices.end(), {bottomLeft, topLeft, bottomRight, bottomRight, topLeft, topRight});
        }
    }
    return indices;
}

std::vector<Vertex> generateUnitCubeVertices()
{
    return {
        // Front
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        // Back
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};
}

std::vector<uint32_t> generateUnitCubeIndices()
{
    return {
        // Front
        0,
        1,
        2,
        0,
        2,
        3,
        // Back
        5,
        4,
        7,
        5,
        7,
        6,
        // Left
        4,
        0,
        3,
        4,
        3,
        7,
        // Right
        1,
        5,
        6,
        1,
        6,
        2,
        // Top
        3,
        2,
        6,
        3,
        6,
        7,
        // Bottom
        4,
        5,
        1,
        4,
        1,
        0};
}
} // namespace reactor
