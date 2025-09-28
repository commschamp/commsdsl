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

#include "commsdsl/gen/GenProgramOptions.h"

#include <iosfwd>
#include <string>
#include <vector>

namespace commsdsl2comms
{

class CommsProgramOptions : public commsdsl::gen::GenProgramOptions
{
public:
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;

    CommsProgramOptions();

    static const CommsProgramOptions& commsCast(const GenProgramOptions& options)
    {
        return static_cast<const CommsProgramOptions&>(options);
    }

    const std::string& commsGetProtocolVersion() const;
    const std::string& commsGetCustomizationLevel() const;
    bool commsVersionIndependentCodeRequested() const;
    std::vector<std::string> commsGetExtraInputBundles() const;
    bool commsIsMainNamespaceInOptionsForced() const;
};

} // namespace commsdsl2comms