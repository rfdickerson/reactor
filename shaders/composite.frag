#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputImage;

// Binding 2: Multisampled depth texture from geometry pass
layout(binding = 2) uniform sampler2DMS uDepthImage;

layout(set = 0, binding = 1) uniform CompositeParams {
    float uExposure;    // Default: 1.0
    float uContrast;    // Default: 1.0
    float uSaturation;  // Default: 1.0
    float uVignetteIntensity;   // Default: 0.5
    float uVignetteFalloff;     // Default 0.5
};

// add camera projection uniforms to the UBO or pass them in another way
const float Z_NEAR = 0.1;
const float Z_FAR = 100.0;

float linearizeDepth(float depth) {
    float z_n = 2.0 * depth - 1.0;
    return (2.0 * Z_NEAR * Z_FAR) / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
}

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

    ivec2 texelCoord = ivec2(gl_FragCoord.xy);

    float depth = texelFetch(uDepthImage, texelCoord, 0).r;

     // --- Fog Calculation ---
        vec3 fogColor = vec3(0.5, 0.6, 0.7); // A cool, hazy blue
        float fogStart = 0.5;
        float fogEnd = 5.0;
        float viewDistance = linearizeDepth(depth);
        // Calculate fog amount (0.0 = no fog, 1.0 = full fog)
        float fogFactor = smoothstep(fogStart, fogEnd, viewDistance);
        // ----------------------

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

    // Mix the final scene color with the fog color
    finalColor = mix(finalColor, fogColor, fogFactor);

    outColor = vec4(finalColor, 1.0);
}