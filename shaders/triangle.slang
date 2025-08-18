import shadows;

// =========================================================================
// 1. Data Structures & Resource Bindings
// =========================================================================

/**
 * Defines the input attributes for a single vertex.
*/
struct VertexInput
{
    float3 position : POSITION;   // Corresponds to GLSL: layout(location = 0) in vec3 inPosition;
    float3 normal   : NORMAL;     // Corresponds to GLSL: layout(location = 1) in vec3 inNormal;
    float3 color    : COLOR;      // Corresponds to GLSL: layout(location = 2) in vec3 inColor;
    float2 texCoord : TEXCOORD0;   // Corresponds to GLSL: layout(location = 3) in vec2 inTexCoord;
};

/**
 * Per-scene constants, equivalent to the `SceneUBO` in GLSL.
 */
[[vk::binding(0, 0)]]
cbuffer SceneUBO : register(b0)
{
    matrix view;
    matrix projection;
    matrix lightSpaceMatrix;
};

/**
 * Per-light constants.
 */
[[vk::binding(1, 0)]]
cbuffer LightUBO : register(b1)
{
    float4 lightDirection;
    float4 lightColor;
    float lightIntensity;
};

/**
 * Push constants for the model matrix.
 */
[push_constant]
cbuffer PushConstants
{
    matrix model;
};

/**
 * Shadow map texture and samplers.
 */
[[vk::binding(2, 0)]]
Texture2D<float> shadowMap          : register(t2);

[[vk::binding(3, 0)]]
SamplerComparisonState shadowSampler : register(s2);

[[vk::binding(4, 0)]]
SamplerState shadowSamplerLinear : register(s3);

// =========================================================================
// 3. Vertex Shader Stage
// =========================================================================

[shader("vertex")]
GeometryVaryings vertexMain(VertexInput input)
{
    GeometryVaryings output;

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
float4 fragmentMain(GeometryVaryings input) : SV_Target
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
    float shadow = calculateShadow(input, shadowMap, shadowSampler, shadowSamplerLinear);

    // Combine components: ambient light is always present, but diffuse
    // light is affected by the shadow factor. 
    float3 finalColor = ambient + (diffuse * shadow);

    return float4(finalColor, 1.0);
}