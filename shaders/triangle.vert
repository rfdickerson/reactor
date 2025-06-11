#version 450

// Vertex index provided by Vulkan (draw calls with vertexCount 3)
void main() {
    // Hardcoded triangle in normalized device coordinates (NDC)
    vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),   // bottom
    vec2(0.5, 0.5),    // right
    vec2(-0.5, 0.5)    // left
    );
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}