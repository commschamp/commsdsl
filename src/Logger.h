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
