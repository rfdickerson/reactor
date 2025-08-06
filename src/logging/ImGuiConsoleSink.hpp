#pragma once

#include "Logger.hpp"

namespace reactor
{

class ImGuiConsoleSink : public ILogSink
{
public:
    explicit ImGuiConsoleSink(size_t maxMessages = 2000);
    void log(const LogMessage& msg) override;

    // Allows the UI thread to safely iterate over the collected messages
    void accessMessages(const std::function<void(const std::deque<LogMessage>&)>& accessor) const;

    // Clears all messages from the console
    void clearMessages();

private:
    std::deque<LogMessage> m_messages;
    size_t m_maxMessages;
    mutable std::mutex m_mutex; // `mutable` allows locking in const methods
};

} // namespace reactor