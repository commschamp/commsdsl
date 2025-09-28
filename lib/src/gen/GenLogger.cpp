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

#include "commsdsl/gen/GenLogger.h"

#include <iostream>
#include <type_traits>

namespace commsdsl
{

namespace gen
{

class GenLoggerImpl
{
public:
    using ParseErrorLevel = GenLogger::ParseErrorLevel;

    void genLog(ParseErrorLevel level, const std::string& msg)
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

    void genSetMinLevel(ParseErrorLevel level)
    {
        m_minLevel = level;
    }

    bool getWarnAsErr() const
    {
        return m_warnAsErr;
    }

    void genSetWarnAsError()
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
    ParseErrorLevel m_minLevel = commsdsl::parse::ParseErrorLevel_Info;
    bool m_warnAsErr = false;
    bool m_hadWarning = false;
};

GenLogger::GenLogger() :
    m_impl(std::make_unique<GenLoggerImpl>())
{
}

GenLogger::~GenLogger() = default;

void GenLogger::genLog(ParseErrorLevel level, const std::string& msg) const
{
    if (level < m_impl->getMinLevel()) {
        return;
    }

    if (m_impl->getWarnAsErr() && (level == commsdsl::parse::ParseErrorLevel_Warning)) {
        m_impl->setHadWarning();
    }

    genLogImpl(level, msg);
}

void GenLogger::genError(const std::string& msg) const
{
    genLog(commsdsl::parse::ParseErrorLevel_Error, msg);
}

void GenLogger::genWarning(const std::string& msg) const
{
    genLog(commsdsl::parse::ParseErrorLevel_Warning, msg);
}

void GenLogger::genInfo(const std::string& msg) const
{
    genLog(commsdsl::parse::ParseErrorLevel_Info, msg);
}

void GenLogger::genDebug(const std::string& msg) const
{
    genLog(commsdsl::parse::ParseErrorLevel_Debug, msg);
}

void GenLogger::genSetMinLevel(ParseErrorLevel level)
{
    m_impl->genSetMinLevel(level);
}

void GenLogger::genSetWarnAsError()
{
    m_impl->genSetWarnAsError();
}

bool GenLogger::genHadWarning() const
{
    return m_impl->getHadWarning();
}

void GenLogger::genLogImpl(commsdsl::parse::ParseErrorLevel level, const std::string& msg) const
{
    m_impl->genLog(level, msg);
}

} // namespace gen

} // namespace commsdsl