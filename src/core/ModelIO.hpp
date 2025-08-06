#pragma once

#include "../vulkan/Vertex.hpp"

namespace reactor
{

// A structure to hold mesh data, which is what we'll load back in the engine.
struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

std::vector<MeshData> loadModelFromBinary(const std::string&);
bool importAndExport(const std::string&, const std::string&);

} // namespace reactor