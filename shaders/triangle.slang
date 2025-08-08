// Combined Slang Shader Module
// triangle.slang

// =========================================================================
// 1. Data Structures & Resource Bindings
// =========================================================================

/**
 * Defines the input attributes for a single vertex.
 * These correspond to the `layout(location = ...)` inputs in the GLSL
 * vertex shader. Semantics like `POSITION` and `NORMAL` are used instead
 * of raw location indices.
 */
struct VertexInput
{
    float3 position : POSITION;   // Corresponds to GLSL: layout(location = 0) in vec3 inPosition;
    float3 normal   : NORMAL;     // Corresponds to GLSL: layout(location = 1) in vec3 inNormal;
    float3 color    : COLOR;      // Corresponds to GLSL: layout(location = 2) in vec3 inColor;
    float2 texCoord : TEXCOORD0;   // Corresponds to GLSL: layout(location = 3) in vec2 inTexCoord;
};

/**
 * Defines the data passed from the vertex shader to the fragment shader.
 * This replaces the individual `out` variables in the vertex shader and
 * `in` variables in the fragment shader. `SV_Position` is a system-value
 * semantic required for the final clip-space position.
 */
struct Varyings
{
    float4 position       : SV_Position; // The final position for the rasterizer.
    float3 worldPos       : WORLD_POS;   // Corresponds to GLSL: outWorldPos
    float3 normal         : NORMAL;      // Corresponds to GLSL: outNormal
    float4 lightSpacePos  : LIGHT_SPACE_POS; // Corresponds to GLSL: outLightSpacePos
};

/**
 * Per-scene constants, equivalent to the `SceneUBO` in GLSL. 
 * The `register(b0)` attribute maps this buffer to binding 0.
 */
[[vk::binding(0, 0)]]
cbuffer SceneUBO : register(b0)
{
    matrix view;
    matrix projection;
    matrix lightSpaceMatrix;
};

/**
 * Per-light constants, equivalent to the `LightUBO` in GLSL. 
 * The `register(b1)` attribute maps this buffer to binding 1.
 */
[[vk::binding(1, 0)]]
cbuffer LightUBO : register(b1)
{
    float4 lightDirection;
    float4 lightColor;
    float lightIntensity;
};

/**
 * Push constants for per-draw call data, like the model matrix.
 */
[push_constant]
cbuffer PushConstants
{
    matrix model;
};

/**
 * In GLSL, `sampler2DShadow` is a combined type. In Slang (and HLSL),
 * the texture and its sampler state are typically defined separately for
 * greater flexibility. We bind them to the same slot index `2` but on
 * different resource types (`t` for texture, `s` for sampler).
 */
[[vk::binding(2, 0)]]
Texture2D<float> shadowMap          : register(t2);

[[vk::binding(3, 0)]]
SamplerComparisonState shadowSampler : register(s2);


// =========================================================================
// 2. Shadow Calculation
// =========================================================================

/**
 * Calculates the shadow contribution. This logic is identical to the
 * GLSL `calculateShadow` function, but it now takes the light-space
 * position as a parameter instead of reading it from a global input.
 */
float calculateShadow(Varyings input)
{
    // Perform perspective divide 
    float3 projCoords = input.lightSpacePos.xyz / input.lightSpacePos.w;

    // Convert from [-1, 1] to [0, 1] texture coordinates 
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Sample the shadow map using a comparison sampler.
    // The `SampleCmp` function performs the depth test (projCoords.z)
    // against the value in the shadow map at projCoords.xy.
    // This is the Slang equivalent of `texture(shadowMap, projCoords)`. 
    return shadowMap.SampleCmp(shadowSampler, projCoords.xy, projCoords.z);
}


// =========================================================================
// 3. Vertex Shader Stage
// =========================================================================

[shader("vertex")]
Varyings vertexMain(VertexInput input)
{
    Varyings output;

    // Transform position and normal to world space 
    float4 worldPos = mul(model, float4(input.position, 1.0));
    output.worldPos = worldPos.xyz; 
    output.normal = normalize(mul((float3x3)model, input.normal));

    // Calculate the final clip-space position 
    output.position = mul(projection, mul(view, worldPos));

    // Transform world position to light's view space for shadowing 
    output.lightSpacePos = mul(lightSpaceMatrix, worldPos);

    return output;
}


// =========================================================================
// 4. Fragment Shader Stage
// =========================================================================

[shader("fragment")]
float4 fragmentMain(Varyings input) : SV_Target
{
    float3 objectColor = float3(0.8, 0.8, 0.8);

    // Ambient light component
    float3 ambient = objectColor * 0.1;

    // Calculate the diffuse (directional) light component 
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(-lightDirection.xyz);
    float diffuseFactor = max(dot(normal, lightDir), 0.0); 
    float3 diffuse = objectColor * lightColor.rgb * lightIntensity * diffuseFactor;

    // Get the shadow factor (1.0 for lit, < 1.0 for shadowed) 
    float shadow = calculateShadow(input);

    // Combine components: ambient light is always present, but diffuse
    // light is affected by the shadow factor. 
    float3 finalColor = ambient + (diffuse * shadow);

    return float4(finalColor, 1.0);
}