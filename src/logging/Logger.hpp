#pragma once

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <deque>

// Use the fmt library for powerful and safe string formatting
#include <fmt/core.h>

namespace reactor
{

// Defines the severity of a log message
enum class LogLevel
{
    Trace,
    Info,
    Warn,
    Error,
    Critical
};

// Represents a single log entry
struct LogMessage
{
    std::string message;
    LogLevel level;
};

// Abstract interface for a "sink" that receives log messages.
class ILogSink
{
  public:
    virtual ~ILogSink() = default;
    virtual void log(const LogMessage& msg) = 0;
};

// The main Logger class, implemented as a singleton for global access.
class Logger
{
  public:
    // Make the singleton non-copyable and non-movable
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // Access the single instance of the logger
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }

    // Attach a new sink to the logger
    void addSink(std::shared_ptr<ILogSink> sink)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sinks.push_back(std::move(sink));
    }

    // Template functions for logging with various arguments
    template <typename... Args>
    void trace(fmt::format_string<Args...> format, Args&&... args)
    {
        log(LogLevel::Trace, fmt::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void info(fmt::format_string<Args...> format, Args&&... args)
    {
        log(LogLevel::Info, fmt::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void warn(fmt::format_string<Args...> format, Args&&... args)
    {
        log(LogLevel::Warn, fmt::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void error(fmt::format_string<Args...> format, Args&&... args)
    {
        log(LogLevel::Error, fmt::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void critical(fmt::format_string<Args...> format, Args&&... args)
    {
        log(LogLevel::Critical, fmt::format(format, std::forward<Args>(args)...));
    }

  private:
    // Private constructor for singleton pattern
    Logger() = default;
    ~Logger() = default;

    // The core log function that distributes messages to sinks
    void log(LogLevel level, const std::string& msg)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        LogMessage logMsg{msg, level};
        for (const auto& sink : m_sinks)
        {
            if (sink)
            {
                sink->log(logMsg);
            }
        }
    }

    std::vector<std::shared_ptr<ILogSink>> m_sinks;
    std::mutex m_mutex;
};

} // namespace reactor

// Helper macros to make logging calls cleaner and less verbose
#define LOG_TRACE(...) reactor::Logger::getInstance().trace(__VA_ARGS__)
#define LOG_INFO(...) reactor::Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARN(...) reactor::Logger::getInstance().warn(__VA_ARGS__)
#define LOG_ERROR(...) reactor::Logger::getInstance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) reactor::Logger::getInstance().critical(__VA_ARGS__)