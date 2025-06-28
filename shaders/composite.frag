#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputImage;

layout(set = 0, binding = 1) uniform CompositeParams {
    float uExposure;
};

// Reinhard tone mapping
vec3 tonemapReinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

void main() {
    // Sample the HDR input image
    vec3 hdrColor = texture(uInputImage, fragUV).rgb;

    // Apply exposure
    vec3 exposedColor = hdrColor * uExposure;

    // Tonemap (Reinhard)
    vec3 ldrColor = tonemapReinhard(exposedColor);

    outColor = vec4(ldrColor, 1.0);
}