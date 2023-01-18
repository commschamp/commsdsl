//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/CommsdslApi.h"

#include <memory>
#include <string>
#include <vector>

namespace commsdsl
{

namespace gen
{

class ProgramOptionsImpl;
class COMMSDSL_API ProgramOptions
{
public:
    using ArgsList = std::vector<std::string>;
    
    ProgramOptions();
    ~ProgramOptions();

    ProgramOptions& addHelpOption();
    ProgramOptions& operator()(const std::string& optStr, const std::string& desc, bool hasParam = false);
    ProgramOptions& operator()(const std::string& optStr, const std::string& desc, const std::string& defaultValue);

    void parse(int argc, const char** argv);
    bool isOptUsed(const std::string& optStr) const;
    bool helpRequested() const;
    const std::string& value(const std::string& optStr) const;
    const ArgsList& args() const;
    std::string helpStr() const;

private:
    std::unique_ptr<ProgramOptionsImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
