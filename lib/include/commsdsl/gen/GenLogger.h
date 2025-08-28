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

#include <string>
#include <memory>

namespace commsdsl
{

namespace gen
{

class GenLoggerImpl;
class GenLogger
{
public:
    using ParseErrorLevel = commsdsl::parse::ParseErrorLevel;
    
    GenLogger();
    GenLogger(const GenLogger&) = delete;
    virtual ~GenLogger();

    void genLog(ParseErrorLevel level, const std::string& msg) const;

    void genError(const std::string& msg) const;
    void genWarning(const std::string& msg) const;
    void genInfo(const std::string& msg) const;
    void genDebug(const std::string& msg) const;
    void genSetMinLevel(ParseErrorLevel level);
    void genSetWarnAsError();
    bool genHadWarning() const;

protected:
    virtual void genLogImpl(ParseErrorLevel level, const std::string& msg) const;    

private:
    mutable std::unique_ptr<GenLoggerImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl