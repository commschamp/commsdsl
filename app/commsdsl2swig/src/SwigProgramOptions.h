//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2swig
{

class SwigProgramOptions : public commsdsl::gen::GenProgramOptions
{
public:
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;

    SwigProgramOptions();

    static const SwigProgramOptions& swigCast(const GenProgramOptions& options)
    {
        return static_cast<const SwigProgramOptions&>(options);
    }

    bool swigIsMainNamespaceInNamesForced() const;
    bool swigHasForcedInterface() const;
    const std::string& swigGetForcedInterface() const;
    bool swigHasCodeVersion() const;
    const std::string& swigMessagesListFile() const;
    const std::string& swigForcedPlatform() const;
};

} // namespace commsdsl2swig