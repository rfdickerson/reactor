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
    // A cube has 6 faces, and each face has 4 vertices.
    // To have sharp edges with distinct normals for each face,
    // we need 24 vertices in total, not 8.
    // The vertex order for each face is:
    // Bottom-Left, Bottom-Right, Top-Right, Top-Left
    return {
        // Back face (-Z)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

        // Front face (+Z)
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

        // Left face (-X)
        {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

        // Right face (+X)
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

        // Bottom face (-Y)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

        // Top face (+Y)
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    };
}

std::vector<uint32_t> generateUnitCubeIndices()
{
    // For each face, we define two triangles.
    // The indices are arranged to have a CLOCKWISE winding order
    // when viewed from outside the cube. This is necessary for
    // back-face culling to work correctly.
    // For a quad with vertices 0,1,2,3 (BL, BR, TR, TL),
    // the triangles are (0, 1, 2) and (0, 2, 3).
    return {
        // Back
        0, 1, 2, 0, 2, 3,
        // Front
        4, 5, 6, 4, 6, 7,
        // Left
        8, 9, 10, 8, 10, 11,
        // Right
        12, 13, 14, 12, 14, 15,
        // Bottom
        16, 17, 18, 16, 18, 19,
        // Top
        20, 21, 22, 20, 22, 23};
}
} // namespace reactor
