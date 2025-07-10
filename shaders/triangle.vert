#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 projection;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 vColor;

void main() {
    gl_Position = projection * view * vec4(inPos, 1.0);
    vColor = inColor;
}