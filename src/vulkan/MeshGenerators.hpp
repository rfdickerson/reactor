#pragma once

#include "Vertex.hpp"

namespace reactor
{
std::vector<Vertex> generatePlaneVertices(int subdivisions, float size);
std::vector<uint32_t> generatePlaneIndices(int subdivisions);
std::vector<Vertex> generateUnitCubeVertices();
std::vector<uint32_t> generateUnitCubeIndices();

} // namespace reactor