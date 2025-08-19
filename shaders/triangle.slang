import shadows;

//
// Resource Bindings & Data Structures
//

struct VertexInput
{
    float3 position : POSITION;  
    float3 normal   : NORMAL;   
    float3 color    : COLOR;  
    float2 texCoord : TEXCOORD0; 
};

[[vk::binding(0, 0)]] cbuffer SceneUBO
{
    float4x4 view;
    float4x4 projection;
    float4x4 lightSpaceMatrix;
};

[[vk::binding(1, 0)]] cbuffer LightUBO
{
    float4 lightDirection;
    float4 lightColor;
    float lightIntensity;
};

[push_constant] cbuffer PushConstants
{
    float4x4 model;
    float4 objectColor;
};

[[vk::binding(2, 0)]] Texture2D<float> shadowMap;
[[vk::binding(3, 0)]] SamplerComparisonState shadowSampler;
[[vk::binding(4, 0)]] SamplerState shadowSamplerLinear;

public interface IBRDF
{
    float3 evaluate(float3 normal, float3 lightDir, float3 viewDir, float3 color);
}

public struct LambertBRDF : IBRDF
{
    float3 evaluate(float3 normal, float3 lightDir, float3 viewDir, float3 color)
    {
        float NdotL = max(dot(normal, lightDir), 0.0);
        return color * NdotL;
    }
};

float3 getLightDir() { return normalize(-lightDirection.xyz); }

// Helper functions
float3 computeDiffuse(float3 normal, float3 objectColor)
{
    float3 lightDir = normalize(-lightDirection.xyz);
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    return objectColor * lightColor.rgb * lightIntensity * diffuseFactor;
}

float3 computeAmbient(float3 objectColor)
{
    return objectColor * 0.1;
}

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
    output.position = mul(projection, mul(view, worldPos));
    output.lightSpacePos = mul(lightSpaceMatrix, worldPos);
    return output;
}

// Fragment Shader

[shader("fragment")]
float4 fragmentMain(GeometryVaryings input) : SV_Target
{
    float3 objectColor = objectColor.rgb;
    float3 ambient = computeAmbient(objectColor);

    LambertBRDF brdf;
    float3 lightDir = getLightDir();
    float3 viewDir = float3(0,0,1);

    float3 diffuse = brdf.evaluate(input.normal, lightDir, viewDir, objectColor) * lightColor.rgb * lightIntensity;

    float shadow = calculateShadow(input, shadowMap, shadowSampler, shadowSamplerLinear);

    float3 finalColor = ambient + (diffuse * shadow);

    return float4(finalColor, 1.0);
}