#pragma once
#include <glm/glm.hpp>

struct SceneUBO {
    glm::mat4 view;
    glm::mat4 projection;
};

struct LightingUBO {
    glm::vec4 lightPosition;
    glm::vec4 lightColor;
    float lightIntensity;
};

struct CompositeUBO {
    float uExposure = 1.0f;
    float uContrast = 1.0f;
    float uSaturation = 1.0f;
    float uVignetteIntensity = 0.5f;
    float uVignetteFalloff = 0.5f;
};