#include "Camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace reactor {

Camera::Camera(): m_view(glm::mat4(1.0f)), m_projection(glm::mat4(1.0f)) {
    updateProjection();
    updateView();
}

void Camera::setProjectionType(ProjectionType type) {
    if (m_type != type) {
        m_type = type;
        m_projDirty = true;
    }
}

void Camera::setPerspective(float fov, float aspect, float near, float far) {
    m_fov = fov;
    m_aspect = aspect;
    m_near = near;
    m_far = far;
    setProjectionType(ProjectionType::Perspective);
    m_projDirty = true;
}

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
    m_viewDirty = true;
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    m_viewDirty = true;
}

void Camera::setUp(const glm::vec3& up) {
    m_up = up;
    m_viewDirty = true;
}

void Camera::lookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up) {
    m_position = eye;
    m_target = target;
    m_up = up;
    m_viewDirty = true;
}

void Camera::move(const glm::vec3& delta) {
    m_position += delta;
    m_target += delta;
    m_viewDirty = true;
}

void Camera::rotate(float yaw, float pitch, float roll) {
    // This offers a very simple orbital rotation around the target point for demonstration.
    // For proper FPS/third-person/free camera, this should be replaced with a full quaternion-based approach.

    glm::vec3 dir = m_target - m_position;
    glm::mat4 rot = glm::eulerAngleYXZ(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));

    dir = glm::vec3(rot * glm::vec4(dir, 0.0f));
    m_position = m_target - dir;
    m_up = glm::vec3(rot * glm::vec4(m_up, 0.0f));
    m_viewDirty = true;
}

// --- Matrix getters ---

const glm::mat4& Camera::getViewMatrix() const {
    if (m_viewDirty) {
        const_cast<Camera*>(this)->updateView();
    }
    return m_view;
}

const glm::mat4& Camera::getProjectionMatrix() const {
    if (m_projDirty) {
        const_cast<Camera*>(this)->updateProjection();
    }
    return m_projection;
}

glm::vec3 Camera::getPosition() const {
    return m_position;
}


void Camera::updateView() {
    m_view = glm::lookAt(m_position, m_target, m_up);
    m_viewDirty = false;
}

void Camera::updateProjection() {
    if (m_type == ProjectionType::Perspective) {
        m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    } else {
        m_projection = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
    }
    m_projDirty = false;
}

} // reactor