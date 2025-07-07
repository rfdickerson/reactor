#include "EventManager.hpp"

namespace reactor {

void EventManager::subscribe(EventType type, IEventListener *listener) {
    m_listeners[type].push_back(listener);
}

void EventManager::unsubscribe(EventType type, IEventListener *listener) {
    auto& listeners = m_listeners[type];
    std::erase(listeners, listener);
}

void EventManager::post(const Event &event) const {
    if (!m_listeners.contains(event.type)) {
        // no listeners, so nothing to do.
        return;
    }

    for (auto listener : m_listeners.at(event.type)) {
        listener->onEvent(event);
    }

}



}