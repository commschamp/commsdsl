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
    SwigProgramOptions();

    bool swigQuietRequested() const;
    bool swigVersionRequested() const;
    bool swigWarnAsErrRequested() const;

    const std::string& swigGetFilesListFile() const;
    const std::string& swigGetFilesListPrefix() const;
    const GenArgsList& swigGetFiles() const;
    const std::string& swigGetOutputDirectory() const;
    const std::string& swigGetCodeInputDirectory() const;
    bool swigHasNamespaceOverride() const;
    const std::string& swigGetNamespace() const;
    bool swigMultipleSchemasEnabled() const;
    unsigned swigGetMinRemoteVersion() const;
    bool swigIsMainNamespaceInNamesForced() const;
    bool swigHasForcedInterface() const;
    const std::string& swigGetForcedInterface() const;
    bool swigHasProtocolVersion() const;
    const std::string& swigMessagesListFile() const;
    const std::string& swigForcedPlatform() const;
};

} // namespace commsdsl2swig