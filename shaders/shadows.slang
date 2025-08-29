module shadows;

[[vk::binding(5, 0)]]
cbuffer ShadowUBO : register(b2)
{
    float lightRadiusUV;
    int   blockerSearchSamples;
    int   pcfSamples;
    float depthBias;
}


public struct GeometryVaryings
{
    public float4 position       : SV_Position;
    public float3 worldPos       : WORLD_POS;
    public float3 normal         : NORMAL;
    public float4 lightSpacePos  : LIGHT_SPACE_POS;
};

static const int kPoissonCount = 32;
static const float2 kPoisson[kPoissonCount] = {
    float2(-0.613392,  0.617481), float2( 0.170019, -0.040254),
    float2(-0.299417,  0.791925), float2( 0.645680,  0.493210),
    float2(-0.651784, -0.634790), float2( 0.421003,  0.027070),
    float2(-0.817194, -0.271096), float2(-0.705374,  0.170919),
    float2( 0.977050, -0.108615), float2( 0.063326,  0.142369),
    float2( 0.203528,  0.214331), float2(-0.667531,  0.326090),
    float2(-0.098422, -0.295755), float2(-0.885922,  0.215369),
    float2( 0.566637,  0.605213), float2( 0.039766, -0.396100),
    float2( 0.751946,  0.453352), float2( 0.078707, -0.715323),
    float2(-0.075838, -0.529344), float2( 0.724479, -0.580798),
    float2( 0.222999, -0.215125), float2(-0.467574, -0.405438),
    float2(-0.248268, -0.814753), float2( 0.354411, -0.887570),
    float2( 0.175817,  0.382366), float2( 0.487472, -0.063082),
    float2(-0.084078,  0.898312), float2( 0.488876, -0.783441),
    float2( 0.470016,  0.217933), float2(-0.696890, -0.549791),
    float2(-0.149693,  0.605762), float2( 0.034211,  0.979980)
};


// float findAverageBlockerDepth(
//     Texture2D<float> shadowMap,
//     SamplerState shadowSamplerLinear,
//     float2 baseUV,
//     float zReceiver)
// {
//     float avgBlockerDepth = 0.0;
//     float numBlockers = 0.0;
//     for (int i = 0; i < blockerSearchSamples; ++i)
//     {
//         float2 offset = kPoisson[i] * lightRadiusUV;
//         float shadowMapDepth = shadowMap.Sample(shadowSamplerLinear, baseUV + offset).r;

//         if (shadowMapDepth < zReceiver)
//         {
//             avgBlockerDepth += shadowMapDepth;
//             numBlockers++;
//         }
//     }

//     return numBlockers > 0.0 ? avgBlockerDepth / numBlockers : 0.0;
// }


float pcfFilter(
    Texture2D<float> shadowMap,
    SamplerComparisonState shadowSampler,
    float2 baseUV,
    float zReceiver,
    float filterRadiusUV)
{
    float shadow = 0.0;
    for (int i = 0; i < pcfSamples; ++i)
    {
        float2 offset = kPoisson[i] * filterRadiusUV;
        shadow += shadowMap.SampleCmp(shadowSampler, baseUV + offset, zReceiver);
    }
    return shadow / float(pcfSamples);
}


public float calculateShadow(
    GeometryVaryings input,
    Texture2D<float> shadowMap,
    SamplerComparisonState shadowSampler,
    SamplerState shadowSamplerLinear)
{
    float3 projCoords = input.lightSpacePos.xyz / input.lightSpacePos.w;
    float2 uv = projCoords.xy * 0.5 + 0.5;
    float zReceiver = projCoords.z - depthBias;

    if (uv.x <= 0.0 || uv.x >= 1.0 || uv.y <= 0.0 || uv.y >= 1.0)
        return 1.0;

    return pcfFilter(shadowMap, shadowSampler, uv, zReceiver, lightRadiusUV);

}