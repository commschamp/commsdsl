#pragma once

#include <string>

#include "commsdsl/ErrorLevel.h"

namespace commsdsl2comms
{

class Logger
{
public:
    Logger() = default;
    Logger(const Logger&) = delete;

    void log(commsdsl::ErrorLevel level, const std::string& msg);
    void error(const std::string& msg)
    {
        log(commsdsl::ErrorLevel_Error, msg);
    }

    void warning(const std::string& msg)
    {
        log(commsdsl::ErrorLevel_Warning, msg);
    }

    void info(const std::string& msg)
    {
        log(commsdsl::ErrorLevel_Info, msg);
    }

    void setMinLevel(commsdsl::ErrorLevel level)
    {
        m_minLevel = level;
    }

    void setWarnAsError()
    {
        m_warnAsErr = true;
    }

    bool hadWarning() const
    {
        return m_hadWarning;
    }

private:
    commsdsl::ErrorLevel m_minLevel = commsdsl::ErrorLevel_Info;
    bool m_warnAsErr = false;
    bool m_hadWarning = false;
};

} // namespace commsdsl2comms
