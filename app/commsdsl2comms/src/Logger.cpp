//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "Logger.h"

#include <type_traits>
#include <iostream>

namespace commsdsl2comms
{

void Logger::log(commsdsl::ErrorLevel level, const std::string& msg)
{
    if (level < m_minLevel) {
        return;
    }

    if (m_warnAsErr && (level == commsdsl::ErrorLevel_Warning)) {
        m_hadWarning = true;
    }

    static const std::string PrefixMap[] = {
        "[DEBUG]: ",
        "[INFO]: ",
        "[WARNING]: ",
        "[ERROR]: "
    };

    static const std::size_t PrefixMapSize = std::extent<decltype(PrefixMap)>::value;

    static_assert(PrefixMapSize == commsdsl::ErrorLevel_NumOfValues, "Wrong map");
    if (commsdsl::ErrorLevel_NumOfValues <= level) {
        level = commsdsl::ErrorLevel_Error;
    }

    std::ostream* stream = &std::cerr;
    if (level < commsdsl::ErrorLevel_Warning) {
        stream = &std::cout;
    }

    *stream << PrefixMap[level] << msg << std::endl;
}

} // namespace commsdsl2comms
