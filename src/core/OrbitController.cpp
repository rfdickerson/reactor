//
// Created by rfdic on 7/7/2025.
//

#include "OrbitController.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace reactor {

 OrbitController::OrbitController(Camera &camera) : m_camera(camera) {}

void OrbitController::setSimCityView(float initialDistance, float initialElevationDegrees) {
     m_distance = initialDistance;
     m_elevation = glm::radians(initialElevationDegrees);
     updateCamera();
 }


void OrbitController::onEvent(const Event &event) {
     switch (event.type) {
         case EventType::MouseMoved:
         if (m_rotating) {
             double dx = event.mouseMove.x - m_lastX;
             double dy = event.mouseMove.y - m_lastY;
             m_azimuth += static_cast<float>(dx) * ROTATE_SPEED;
             m_elevation += static_cast<float>(dy) * ROTATE_SPEED;
             m_elevation = glm::clamp(m_elevation, MIN_ELEVATION, MAX_ELEVATION);
             updateCamera();
             m_lastX = event.mouseMove.x;
             m_lastY = event.mouseMove.y;
         }
         if (m_panning) {
             double dx = event.mouseMove.x - m_lastPanX;
             double dy = event.mouseMove.y - m_lastPanY;
             updatePan(static_cast<float>(dx), static_cast<float>(dy));
             m_lastPanX = event.mouseMove.x;
             m_lastPanY = event.mouseMove.y;
         }
         break;
     case EventType::MouseButtonPressed:
         if (event.mouseButton.button == 0) {
             m_rotating = true;
             m_lastX = event.mouseButton.x;
             m_lastY = event.mouseButton.y;
         } else if (event.mouseButton.button == 1) {
             m_panning = true;
             m_lastPanX = event.mouseButton.x;
             m_lastPanY = event.mouseButton.y;
         }
         break;
     case EventType::MouseButtonReleased:
         if (event.mouseButton.button == 0) {
             m_rotating = false;
         } else if (event.mouseButton.button == 1) {
             m_panning = false;
         }
         break;
     case EventType::KeyPressed:
         break;
     }
 }

void OrbitController::updateCamera() {
     const float x = m_distance * cosf(m_elevation) * sinf(m_azimuth);
     const float y = m_distance * sinf(m_elevation);
     const float z = m_distance * cosf(m_elevation) * cosf(m_azimuth);

     const glm::vec3     position = m_target + glm::vec3(x, y, z);
     const glm::vec3 up(0.0f, 1.0f, 0.0f);

     m_camera.lookAt(position, m_target, up);
 }

void OrbitController::updatePan(float dx, float dy) {
     const float fovScale = std::tan(glm::radians(m_camera.getFOV() / 2.0f));
     const float speed = (m_distance * fovScale * PAN_SPEED);

     // get camera facing direction
     const glm::vec3 position = m_camera.getPosition();
     const glm::vec3 facingDir = glm::normalize(position - m_target);

     // horizontal right
     const glm::vec3 up(0.0f, 1.0f, 0.0f);
     const glm::vec3 right = glm::normalize(glm::cross(facingDir, up));

     // horizontal forward
     const glm::vec3 groundForward = glm::normalize(glm::vec3(facingDir.x, 0.0f, facingDir.z));

     const glm::vec3 delta = (right * -dx * speed) + (groundForward * -dy * speed);

     m_camera.move(delta);
     m_target += delta;
     updateCamera();

 }


} // reactor