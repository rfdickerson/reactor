#pragma once

#include "EventManager.hpp"
#include <glm/glm.hpp>

namespace reactor {

class Camera {
public:

    Camera();

    [[nodiscard]] glm::mat4 getView() const { return m_view; }
    [[nodiscard]] glm::mat4 getProjection() const { return m_projection; }

    void setView(const glm::mat4& view) { m_view = view; }

private:
    glm::mat4 m_view;
    glm::mat4 m_projection;

};

} // reactor


