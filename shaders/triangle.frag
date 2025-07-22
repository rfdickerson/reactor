#version 450

layout(location = 1) in vec3 vNormal;

// Final output color
layout(location = 0) out vec4 outColor;

void main() {
    // Normalize the incoming normal vector to ensure unit vector.
    vec3 normal = normalize(vNormal);

    // map normal components in (-1, 1) to (0, 1)
    vec3 normalColor = (normal + 1.0) * 0.5;

    outColor = vec4(normalColor, 1.0);
}