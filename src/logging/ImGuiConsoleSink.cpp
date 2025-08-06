#include "ImGuiConsoleSink.hpp"

namespace reactor
{

ImGuiConsoleSink::ImGuiConsoleSink(size_t maxMessages) : m_maxMessages(maxMessages) {}

void ImGuiConsoleSink::log(const LogMessage& msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.push_back(msg);
    // Trim the queue if it exceeds the maximum size
    if (m_messages.size() > m_maxMessages)
    {
        m_messages.pop_front();
    }
}

void ImGuiConsoleSink::accessMessages(const std::function<void(const std::deque<LogMessage>&)>& accessor) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    accessor(m_messages);
}

void ImGuiConsoleSink::clearMessages()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.clear();
}

} // namespace reactor