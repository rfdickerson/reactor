#include "Camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stdexcept>

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
    if (fov <= 0.0f || fov >= 180.0f || aspect <= 0.0f || near <= 0.0f || far <= 0.0f) {
        throw std::invalid_argument("Invalid camera parameters");
    }
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
    glm::vec3 direction = glm::normalize(target - m_position);
    m_orientation = glm::quatLookAt(direction, m_up);
    m_viewDirty = true;
}

void Camera::setUp(const glm::vec3& up) {
    m_up = glm::normalize(up);
    m_viewDirty = true;
}

void Camera::lookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up) {
    m_position = eye;
    setTarget(target);
    m_up = normalize(up);
    m_viewDirty = true;
}

void Camera::move(const glm::vec3& delta) {
    m_position += delta;
    m_viewDirty = true;
}

void Camera::moveRelative(const glm::vec3 &delta) {
    m_position += getRight() * delta.x + getUp() * delta.y + getForward() * delta.z;
    m_viewDirty = true;
}


void Camera::rotate(float yaw, float pitch, float roll) {
    glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qPitch = glm::angleAxis(glm::radians(pitch), getRight());
    glm::quat qRoll = glm::angleAxis(glm::radians(roll), getForward());
    m_orientation = glm::normalize(qYaw * m_orientation * qPitch * qRoll);
    m_up = getUp();
    m_viewDirty = true;
}

void Camera::dolly(float distance) {
    glm::vec3 direction = getForward();
    m_position += direction * distance;
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

float Camera::getFOV() const {
    return m_fov;
}

glm::vec3 Camera::getPosition() const {
    return m_position;
}

glm::vec3 Camera::getTarget() const {
    return m_position + getForward();
}

glm::vec3 Camera::getForward() const {
    return glm::vec3(m_orientation * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 Camera::getRight() const {
    return glm::vec3(m_orientation * glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 Camera::getUp() const {
    return glm::normalize(m_orientation * glm::vec3(0.0f, 1.0f, 0.0f));
}

float Camera::getDistanceToTarget() const {
    return 1.0;
}

void Camera::updateView() {
    glm::mat4 rotation = glm::toMat4(glm::conjugate(m_orientation));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_position);
    m_view = rotation * translation;
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