//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <sstream>

#include "commsdsl/parse/ErrorLevel.h"
#include "commsdsl/parse/Protocol.h"

namespace commsdsl
{

namespace parse
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

} // namespace parse

} // namespace commsdsl

