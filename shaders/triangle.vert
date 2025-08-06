#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outLightSpacePos;

layout(binding = 0) uniform SceneUBO {
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

void main() {
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    gl_Position = ubo.projection * ubo.view * worldPos;

    outWorldPos = worldPos.xyz;
    outNormal = normalize(mat3(push.model) * inNormal);
    outLightSpacePos = ubo.lightSpaceMatrix * worldPos;
}