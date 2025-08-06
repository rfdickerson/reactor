#pragma once

#include "Logger.hpp"
#include <spdlog/spdlog.h>

namespace reactor
{

class SpdlogSink : public ILogSink
{
public:
    SpdlogSink();
    void log(const LogMessage& msg) override;

private:
    std::shared_ptr<spdlog::logger> m_spdlogLogger;
};

} // namespace reactor