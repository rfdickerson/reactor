#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 projection;
};

layout(location = 0) out vec3 vColor;

void main() {
    // Hardcoded triangle in normalized device coordinates (NDC)
    vec3 positions[3] = vec3[](
        vec3(0.0, -0.5, 0.0),   // bottom
        vec3(0.5, 0.5, 0.0),    // right
        vec3(-0.5, 0.5, 0.0)    // left
    );

    vec3 colors[3] = vec3[](
        vec3(1.0, 0.0, 0.0), // Red
        vec3(0.0, 1.0, 0.0), // Green
        vec3(0.0, 0.0, 1.0)  // Blue
    );

    gl_Position = projection * view * vec4(positions[gl_VertexIndex], 1.0);
    vColor = colors[gl_VertexIndex];
}