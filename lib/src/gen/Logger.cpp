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

#include "commsdsl/gen/Logger.h"

#include <type_traits>
#include <iostream>

namespace commsdsl
{

namespace gen
{

class LoggerImpl
{
public:
    using ParseErrorLevel = Logger::ParseErrorLevel;

    void log(ParseErrorLevel level, const std::string& msg)
    {
        static const std::string PrefixMap[] = {
            "[DEBUG]: ",
            "[INFO]: ",
            "[WARNING]: ",
            "[ERROR]: "
        };

        static const std::size_t PrefixMapSize = std::extent<decltype(PrefixMap)>::value;

        static_assert(PrefixMapSize == commsdsl::parse::ParseErrorLevel_NumOfValues, "Wrong map");
        if (commsdsl::parse::ParseErrorLevel_NumOfValues <= level) {
            level = commsdsl::parse::ParseErrorLevel_Error;
        }

        std::ostream* stream = &std::cerr;
        if (level < commsdsl::parse::ParseErrorLevel_Warning) {
            stream = &std::cout;
        }

        *stream << PrefixMap[level] << msg << std::endl;
    }

    ParseErrorLevel getMinLevel() const
    {
        return m_minLevel;
    }

    void setMinLevel(ParseErrorLevel level)
    {
        m_minLevel = level;
    }

    bool getWarnAsErr() const
    {
        return m_warnAsErr;
    }

    void setWarnAsError()
    {
        m_warnAsErr = true;
    }

    void setHadWarning()
    {
        m_hadWarning = true;
    }

    bool getHadWarning() const
    {
        return m_hadWarning;
    }

private:
    commsdsl::parse::ParseErrorLevel m_minLevel = commsdsl::parse::ParseErrorLevel_Info;
    bool m_warnAsErr = false;
    bool m_hadWarning = false;
};

Logger::Logger() : 
    m_impl(std::make_unique<LoggerImpl>())
{
}

Logger::~Logger() = default;

void Logger::log(commsdsl::parse::ParseErrorLevel level, const std::string& msg) const
{
    if (level < m_impl->getMinLevel()) {
        return;
    }

    if (m_impl->getWarnAsErr() && (level == commsdsl::parse::ParseErrorLevel_Warning)) {
        m_impl->setHadWarning();
    }

    logImpl(level, msg);
}

void Logger::error(const std::string& msg) const
{
    log(commsdsl::parse::ParseErrorLevel_Error, msg);
}

void Logger::warning(const std::string& msg) const
{
    log(commsdsl::parse::ParseErrorLevel_Warning, msg);
}

void Logger::info(const std::string& msg) const
{
    log(commsdsl::parse::ParseErrorLevel_Info, msg);
}

void Logger::debug(const std::string& msg) const
{
    log(commsdsl::parse::ParseErrorLevel_Debug, msg);
}

void Logger::setMinLevel(ParseErrorLevel level)
{
    m_impl->setMinLevel(level);
}

void Logger::setWarnAsError()
{
    m_impl->setWarnAsError();
}

bool Logger::hadWarning() const
{
    return m_impl->getHadWarning();
}

void Logger::logImpl(commsdsl::parse::ParseErrorLevel level, const std::string& msg) const
{
    m_impl->log(level, msg);
}

} // namespace gen

} // namespace commsdsl