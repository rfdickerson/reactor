#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace reactor {

enum class ProjectionType {
    Perspective,
    Orthographic,
};

class Camera {
public:
    Camera();

    void setProjectionType(ProjectionType type);
    void setPerspective(float fov, float aspect, float near, float far);

    void setPosition(const glm::vec3& position);
    void setTarget(const glm::vec3& target);
    void setUp(const glm::vec3& up);

    void lookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
    void move(const glm::vec3& delta);
    void moveRelative(const glm::vec3& delta);
    void rotate(float yaw, float pitch, float roll);
    void dolly(float distance);

    // getters
    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getProjectionMatrix() const;
    float getFOV() const;
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;
    float getDistanceToTarget() const;

private:
    void updateView();
    void updateProjection();

    ProjectionType m_type{ProjectionType::Perspective};
    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};

    float m_fov{45.0f}, m_aspect{16.0f / 9.0f}, m_near{0.1f}, m_far{100.0f};
    float m_left, m_right, m_bottom, m_top;

    glm::vec3 m_position{0.0f, 0.0f, 5.0f};
    glm::quat m_orientation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    mutable bool m_viewDirty{true};
    mutable bool m_projDirty{true};
};

} // reactor


