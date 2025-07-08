#pragma once

#include "EventManager.hpp"
#include "Camera.hpp"

namespace reactor {

class OrbitController final : public IEventListener {
public:
    explicit OrbitController(Camera& camera);

    void onEvent(const Event& event) override;

private:
    Camera& m_camera;
    float m_distance = 5.0f;
    float m_azimuth = 0.0f;
    float m_elevation = 0.0f;

    bool m_rotating = false;
    double m_lastX = 0.0;
    double m_lastY = 0.0;

    void updateCamera();

    static constexpr float ROTATE_SPEED = 0.01f;
    static constexpr float ZOOM_SPEED = 0.5f;
    static constexpr float MIN_DISTANCE = 0.1f;
    static constexpr float MAX_DISTANCE = 50.0f;
};

} // reactor

