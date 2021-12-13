//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include <string>

#include "commsdsl/parse/ErrorLevel.h"

namespace commsdsl2comms
{

class Logger
{
public:
    Logger() = default;
    Logger(const Logger&) = delete;

    void log(commsdsl::parse::ErrorLevel level, const std::string& msg);
    void error(const std::string& msg)
    {
        log(commsdsl::parse::ErrorLevel_Error, msg);
    }

    void warning(const std::string& msg)
    {
        log(commsdsl::parse::ErrorLevel_Warning, msg);
    }

    void info(const std::string& msg)
    {
        log(commsdsl::parse::ErrorLevel_Info, msg);
    }

    void setMinLevel(commsdsl::parse::ErrorLevel level)
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
    commsdsl::parse::ErrorLevel m_minLevel = commsdsl::parse::ErrorLevel_Info;
    bool m_warnAsErr = false;
    bool m_hadWarning = false;
};

} // namespace commsdsl2comms