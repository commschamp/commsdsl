#pragma once

#include <sstream>

#include "commsdsl/ErrorLevel.h"
#include "commsdsl/Protocol.h"

namespace commsdsl
{

class ProtocolImpl;

class Logger
{
public:
    using ReportFunc = Protocol::ErrorReportFunction;

    template <typename TFunc>
    Logger(TFunc&& func)
      : m_func(std::forward<TFunc>(func))
    {
    }

    ~Logger() = default;

    void setMinLevel(ErrorLevel val)
    {
        m_minLevel = val;
    }

    void setCurrLevel(ErrorLevel val)
    {
        m_currLevel = val;
    }

    void flush()
    {
        if (m_minLevel <= m_currLevel) {
            m_func(m_currLevel, m_stream.str());
            m_stream = std::stringstream();
        }
    }

    template <typename T>
    Logger& operator<<(T&& val)
    {
        if (m_minLevel <= m_currLevel) {
            m_stream << std::forward<T>(val);
        }
        return *this;
    }

private:
    ErrorLevel m_minLevel = ErrorLevel_Debug;
    ErrorLevel m_currLevel = ErrorLevel_Debug;
    ReportFunc m_func;

    std::stringstream m_stream;
};

class LogWrapper
{
public:
    LogWrapper(Logger& logger) : m_logger(logger) {}
    ~LogWrapper() { m_logger.flush(); }

    template <typename T>
    LogWrapper& operator<<(T&& val)
    {
        m_logger << std::forward<T>(val);
        return *this;
    }

private:
    Logger& m_logger;
};

inline
LogWrapper logError(Logger& logger)
{
    logger.setCurrLevel(ErrorLevel_Error);
    return LogWrapper(logger);
}

inline
LogWrapper logWarning(Logger& logger)
{
    logger.setCurrLevel(ErrorLevel_Warning);
    return LogWrapper(logger);
}

inline
LogWrapper logInfo(Logger& logger)
{
    logger.setCurrLevel(ErrorLevel_Info);
    return LogWrapper(logger);
}

} // namespace commsdsl

