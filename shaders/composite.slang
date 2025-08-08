struct Varyings
{
    float4 position : SV_Position; // System value for clip space position
    float2 uv       : TEXCOORD0;   // The UV coordinates for sampling
};

/**
 * The C++ code in `VulkanRenderer.cpp` creates a multisampled depth
 * image with 4 samples. We explicitly define that here.
 */
static const int MSAA_SAMPLES = 4;

[[vk::binding(0, 0)]]
Texture2D uInputImage : register(t0);

[[vk::binding(3, 0)]]
SamplerState g_sampler : register(s3);

[[vk::binding(2, 0)]]
Texture2D uDepthImage : register(t2);

[[vk::binding(1, 0)]]
cbuffer CompositeParams : register(b1)
{
    float uExposure;
    float uContrast;
    float uSaturation;
    float uVignetteIntensity;
    float uVignetteFalloff;
    float uFogDensity;
};

float linearizeDepth(float depth)
{
    const float Z_NEAR = 0.1;
    const float Z_FAR = 100.0;
    float z_n = 2.0 * depth - 1.0;
    return (2.0 * Z_NEAR * Z_FAR) / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
}

float3 tonemapACES(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

[shader("vertex")]
Varyings vertexMain(uint vertexID : SV_VertexID)
{
    Varyings output;

    // Defines the triangle vertices to cover the screen[cite: 13].
    const float2 pos[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0)
    };
    // Defines the UV coordinates for the triangle[cite: 14].
    const float2 uv[3] = {
        float2(0.0, 0.0),
        float2(2.0, 0.0),
        float2(0.0, 2.0)
    };

    output.uv = uv[vertexID];
    output.position = float4(pos[vertexID], 0.0, 1.0);

    return output;
}

[shader("fragment")]
float4 fragmentMain(Varyings input) : SV_Target
{
    // Get the integer coordinates for the current fragment.
    int2 texelCoord = int2(input.position.xy);

    // To read from a Texture2DMS, use the `.Load()` method.
    // It takes the coordinate and the sample index (we'll just use the first).
    // This replaces `texelFetch(uDepthImage, texelCoord, 0)`.
    float depth = uDepthImage.Sample(g_sampler, input.uv).r;

    // --- Fog Calculation ---
    float3 fogColor = float3(0.5, 0.6, 0.7);
    float viewDistance = linearizeDepth(depth);
    float fogFactor = 1.0 - exp(-viewDistance * uFogDensity);

    // --- Color Processing ---
    // To sample a standard texture, use the `.Sample()` method.
    // This replaces `texture(uInputImage, fragUV)`.
    float3 hdrColor = uInputImage.Sample(g_sampler, input.uv).rgb;

    // Apply exposure, tonemapping, and color grading[cite: 35, 36, 37, 38, 39, 40].
    float3 exposedColor = hdrColor * uExposure;
    float3 ldrColor = tonemapACES(exposedColor);
    float3 finalColor = ldrColor;

    finalColor = pow(finalColor, float3(uContrast));
    float3 grayscale = float3(dot(finalColor, float3(0.299, 0.587, 0.114)));
    finalColor = lerp(grayscale, finalColor, uSaturation);
    finalColor = lerp(finalColor, fogColor, fogFactor);

    return float4(finalColor, 1.0);
}