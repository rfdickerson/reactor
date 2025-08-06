#include "SpdlogSink.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace reactor
{

SpdlogSink::SpdlogSink()
{
    // Initialize a thread-safe color logger that outputs to the console
    m_spdlogLogger = spdlog::stdout_color_mt("console");
}

void SpdlogSink::log(const LogMessage& msg)
{
    // Map our LogLevel to the corresponding spdlog function
    switch (msg.level)
    {
    case LogLevel::Trace:
        m_spdlogLogger->trace(msg.message);
        break;
    case LogLevel::Info:
        m_spdlogLogger->info(msg.message);
        break;
    case LogLevel::Warn:
        m_spdlogLogger->warn(msg.message);
        break;
    case LogLevel::Error:
        m_spdlogLogger->error(msg.message);
        break;
    case LogLevel::Critical:
        m_spdlogLogger->critical(msg.message);
        break;
    }
}

} // namespace reactor