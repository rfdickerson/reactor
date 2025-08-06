#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inLightSpacePos;

layout(binding = 1) uniform LightUBO {
    vec4 lightDirection;
    vec4 lightColor;
    float lightIntensity;
} ubo;

layout(binding = 2) uniform sampler2DShadow shadowMap;

// Final output color
layout(location = 0) out vec4 outColor;

float calculateShadow()
{
    // Perform perspective divide
    vec3 projCoords = inLightSpacePos.xyz / inLightSpacePos.w;

    // Convert from [-1, 1] to [0, 1] texture coordinates
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Get the closest depth from the shadow map (from light's perspective)
    // The 'z' coordinate of projCoords is the current fragment's depth from the light
    // texture() for sampler2DShadow performs the depth comparison automatically
    float shadow = texture(shadowMap, projCoords);

    return shadow;
}

void main() {
    vec3 objectColor = vec3(0.8, 0.8, 0.8);

    vec3 ambient = objectColor * 0.1;

    // 2. Calculate the diffuse (directional) light component.
    vec3 normal = normalize(inNormal);
    vec3 lightDir = normalize(-ubo.lightDirection.xyz);
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = objectColor * ubo.lightColor.rgb * ubo.lightIntensity * diffuseFactor;

    // 3. Get the shadow factor (1.0 for lit, 0.0 for shadowed).
    float shadow = calculateShadow();

    // 4. Combine the components for the final color.
    // The final color is the ambient light, plus the direct (diffuse) light,
    // which IS blocked by shadows.
    vec3 finalColor = ambient + (diffuse * shadow);

    outColor = vec4(finalColor, 1.0);

}