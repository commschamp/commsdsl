//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseErrorLevel.h"
#include "commsdsl/parse/ParseProtocol.h"

#include <sstream>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;

class ParseLogger
{
public:
    using ReportFunc = ParseProtocol::ErrorReportFunction;

    template <typename TFunc>
    ParseLogger(TFunc&& func)
      : m_func(std::forward<TFunc>(func))
    {
    }

    ~ParseLogger() = default;

    void parseSetMinLevel(ParseErrorLevel val)
    {
        m_minLevel = val;
    }

    void parseSetCurrLevel(ParseErrorLevel val)
    {
        m_currLevel = val;
    }

    void parseFlush()
    {
        if (m_minLevel <= m_currLevel) {
            m_func(m_currLevel, m_stream.str());
            m_stream = std::stringstream();
        }
    }

    template <typename T>
    ParseLogger& operator<<(T&& val)
    {
        if (m_minLevel <= m_currLevel) {
            m_stream << std::forward<T>(val);
        }
        return *this;
    }

private:
    ParseErrorLevel m_minLevel = ParseErrorLevel_Debug;
    ParseErrorLevel m_currLevel = ParseErrorLevel_Debug;
    ReportFunc m_func;

    std::stringstream m_stream;
};

class LogWrapper
{
public:
    LogWrapper(ParseLogger& logger) : m_logger(logger) {}
    ~LogWrapper() { m_logger.parseFlush(); }

    template <typename T>
    LogWrapper& operator<<(T&& val)
    {
        m_logger << std::forward<T>(val);
        return *this;
    }

private:
    ParseLogger& m_logger;
};

inline
LogWrapper parseLogError(ParseLogger& logger)
{
    logger.parseSetCurrLevel(ParseErrorLevel_Error);
    return LogWrapper(logger);
}

inline
LogWrapper parseLogWarning(ParseLogger& logger)
{
    logger.parseSetCurrLevel(ParseErrorLevel_Warning);
    return LogWrapper(logger);
}

inline
LogWrapper parseLogInfo(ParseLogger& logger)
{
    logger.parseSetCurrLevel(ParseErrorLevel_Info);
    return LogWrapper(logger);
}

} // namespace parse

} // namespace commsdsl

