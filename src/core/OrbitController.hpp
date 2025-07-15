#pragma once

#include "EventManager.hpp"
#include "Camera.hpp"

namespace reactor {

class OrbitController final : public IEventListener {
public:
    explicit OrbitController(Camera& camera);

    void onEvent(const Event& event) override;

    void setSimCityView(float initialDistance = 20.f, float initialElevationDegrees = 45.0f);

private:
    Camera& m_camera;
    glm::vec3 m_target{0.0f};
    float m_distance = 5.0f;
    float m_azimuth = 0.0f;
    float m_elevation = 0.0f;

    bool m_rotating = false;
    bool m_panning = false;
    double m_lastX = 0.0;
    double m_lastY = 0.0;
    double m_lastPanX = 0.0;
    double m_lastPanY = 0.0;

    void updateCamera();
    void updatePan(float dx, float dy);

    static constexpr float ROTATE_SPEED = 0.01f;
    static constexpr float PAN_SPEED = 0.0005f;
    static constexpr float ZOOM_SPEED = 0.5f;
    static constexpr float MIN_DISTANCE = 0.1f;
    static constexpr float MAX_DISTANCE = 50.0f;
    static constexpr float MIN_ELEVATION = glm::radians(20.0f);
    static constexpr float MAX_ELEVATION = glm::radians(85.0f);
};

} // reactor

