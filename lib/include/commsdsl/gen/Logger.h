//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ErrorLevel.h"

#include <string>
#include <memory>

namespace commsdsl
{

namespace gen
{

class LoggerImpl;
class Logger
{
public:
    using ErrorLevel = commsdsl::parse::ErrorLevel;
    Logger();
    Logger(const Logger&) = delete;
    virtual ~Logger();

    void log(ErrorLevel level, const std::string& msg) const;

    void error(const std::string& msg) const;
    void warning(const std::string& msg) const;
    void info(const std::string& msg) const;
    void debug(const std::string& msg) const;
    void setMinLevel(ErrorLevel level);
    void setWarnAsError();
    bool hadWarning() const;

protected:
    virtual void logImpl(ErrorLevel level, const std::string& msg) const;    

private:
    mutable std::unique_ptr<LoggerImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl