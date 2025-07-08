#pragma once
#include <unordered_map>
#include <vector>

namespace reactor {

enum class EventType {
    MouseMoved,
    MouseButtonPressed,
    MouseButtonReleased,
    KeyPressed,
    KeyReleased,
};

struct Event {
    EventType type;
    union {
        struct {
            double x, y;
        } mouseMove;
        struct {
            int button;
            double x, y;
        } mouseButton;
        struct {
            int key;
        } keyboard;
    };
};

class IEventListener {
public:
    virtual ~IEventListener() = default;
    virtual void onEvent(const Event& event) = 0;
};

class EventManager {
public:
    void subscribe(EventType type, IEventListener* listener);
    void unsubscribe(EventType type, IEventListener* listener);
    void post(const Event& event) const;

private:
    std::unordered_map<EventType, std::vector<IEventListener*>> m_listeners;
};

}