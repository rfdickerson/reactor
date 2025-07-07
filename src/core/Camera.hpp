#pragma once

#include "EventManager.hpp"
#include <glm/glm.hpp>

namespace reactor {

class Camera final : public IEventListener {
public:

    Camera();

    void onEvent(const Event& event) override;

    [[nodiscard]] glm::mat4 getView() const { return m_view; }
    [[nodiscard]] glm::mat4 getProjection() const { return m_projection; }

private:
    glm::mat4 m_view;
    glm::mat4 m_projection;

};

} // reactor


