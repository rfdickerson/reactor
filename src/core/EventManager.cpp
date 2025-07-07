#include "EventManager.hpp"

namespace reactor {

void EventManager::subscribe(EventType type, IEventListener *listener) {
    m_listeners[type].push_back(listener);
}

void EventManager::unsubscribe(EventType type, IEventListener *listener) {
    auto& listeners = m_listeners[type];
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

void EventManager::post(const Event &event) {
    if (m_listeners.find(event.type) == m_listeners.end()) {
        // no listeners, so nothing to do.
        return;
    }

    for (auto listener : m_listeners.at(event.type)) {
        listener->onEvent(event);
    }

}



}