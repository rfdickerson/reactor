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

    // Define a base color for object
    vec3 objectColor = vec3(0.8, 0.8, 0.8);

    // Ensure the normal is a unit vector
    vec3 normal = normalize(inNormal);

    // The direction vector from the uniform is pointing *to* the light.
    // We use it directly for the dot product.
    vec3 lightDir = normalize(ubo.lightDirection.xyz);

    // Calculate the diffuse factor (Lambertian reflectance)
    // max(..., 0.0) ensures that we don't have negative light
    float diffuseFactor = max(dot(normal, lightDir), 0.0);

    // Calculate the final diffuse light color
    vec3 diffuse = ubo.lightColor.rgb * ubo.lightIntensity * diffuseFactor;

    // Calculate shadow factor
    float shadow = calculateShadow();

    // The final color is the object's base color modulated by the diffuse light and shadow
    vec3 finalColor = objectColor * diffuse * shadow;

    finalColor += objectColor * 0.1;

    // The final color is the object's base color modulated by the diffuse light
    outColor = vec4(finalColor, 1.0);

}