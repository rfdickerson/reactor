#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputImage;

layout(set = 0, binding = 1) uniform CompositeParams {
    float uExposure;    // Default: 1.0
    float uContrast;    // Default: 1.0
    float uSaturation;  // Default: 1.0
    float uVignetteIntensity;   // Default: 0.5
    float uVignetteFalloff;     // Default 0.5
};

// ACES Filmic Tone Mapping Curve
vec3 tonemapACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    // Sample the HDR input image
    vec3 hdrColor = texture(uInputImage, fragUV).rgb;

    // Apply exposure
    vec3 exposedColor = hdrColor * uExposure;

    // Tonemap (Reinhard)
    vec3 ldrColor = tonemapACES(exposedColor);

    // Apply Color Grading
    vec3 finalColor = ldrColor;

    // Contrast
    finalColor = pow(finalColor, vec3(uContrast));

    // Saturation
    vec3 grayscale = vec3(dot(finalColor, vec3(0.299, 0.587, 0.114)));
    finalColor = mix(grayscale, finalColor, uSaturation);

    outColor = vec4(finalColor, 1.0);
}