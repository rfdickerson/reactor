#pragma once
#include <glm/glm.hpp>

struct SceneUBO {
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 lightSpaceMatrix;
};

struct DirectionalLightUBO
{
    glm::vec4 lightPosition = glm::vec4(-0.5f, 1.0f, -0.5f, 0.0f);
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float lightIntensity = 1.0f;
    // add 12-bytes of padding
    float pad[3];
};

struct CompositeUBO {
    float uExposure = 1.0f;
    float uContrast = 1.0f;
    float uSaturation = 1.0f;
    float uVignetteIntensity = 0.5f;
    float uVignetteFalloff = 0.5f;
    float uFogDensity = 0.001f;
};