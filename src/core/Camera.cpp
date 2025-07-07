//
// Created by rfdic on 7/7/2025.
//

#include "Camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <spdlog/spdlog.h>

namespace reactor {

Camera::Camera(): m_view(glm::mat4(1.0f)), m_projection(glm::mat4(1.0f)) {
    m_projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
}

void Camera::onEvent(const Event &event) {
    // handle moving camera
    spdlog::info("Camera event");
}

} // reactor