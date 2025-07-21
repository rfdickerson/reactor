#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(binding = 0) uniform SceneUBO {
    mat4 view;
    mat4 projection;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.projection * ubo.view * push.model * vec4(inPos, 1.0);
    fragNormal = mat3(transpose(inverse(push.model))) * inNormal;  // Correct for non-uniform scales
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}