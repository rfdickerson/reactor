#version 450

// input from vertex shader
layout(location = 1) in vec3 inNormal;

// Uniform buffer for lighting data
layout(binding = 1) uniform LightUBO {
    vec4 lightDirection;
    vec4 lightColor;
    float lightIntensity;
} ubo;

// Final output color
layout(location = 0) out vec4 outColor;

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

    // The final color is the object's base color modulated by the diffuse light
    outColor = vec4(objectColor * diffuse, 1.0);

}