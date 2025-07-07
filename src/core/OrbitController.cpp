//
// Created by rfdic on 7/7/2025.
//

#include "OrbitController.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace reactor {

 OrbitController::OrbitController(Camera &camera) : m_camera(camera) {}

void OrbitController::onEvent(const Event &event) {
     switch (event.type) {
         case EventType::MouseMoved:
         if (m_rotating) {
             double dx = event.mouseMove.x - m_lastX;
             double dy = event.mouseMove.y - m_lastY;
             m_azimuth += static_cast<float>(dx) * ROTATE_SPEED;
             m_elevation += static_cast<float>(dy) * ROTATE_SPEED;
             // clamp elevation
             float max_elev = glm::radians(85.0f);
             m_elevation = glm::clamp(m_elevation, -max_elev, max_elev);
             updateCamera();
         }
         m_lastX = event.mouseMove.x;
         m_lastY = event.mouseMove.y;
         break;
     case EventType::KeyPressed:
         break;
         case EventType::KeyReleased:
         break;

         }
 }

void OrbitController::updateCamera() {
     const float x = m_distance * cosf(m_elevation) * sinf(m_azimuth);
     const float y = m_distance * sinf(m_elevation);
     const float z = m_distance * cosf(m_elevation) * cosf(m_azimuth);

     const glm::vec3     position(x, y, z);
     constexpr glm::vec3 target(0.0f);
     constexpr glm::vec3 up(0.0f, 1.0f, 0.0f);

     m_camera.setView(glm::lookAt(position, target, up));
 }


} // reactor