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

#include <vector>
#include <string>
#include <iosfwd>

#include "commsdsl/gen/GenProgramOptions.h"

namespace commsdsl2comms
{

class CommsProgramOptions : public commsdsl::gen::GenProgramOptions
{
public:
    CommsProgramOptions();

    bool quietRequested() const;
    bool debugRequested() const;
    bool versionRequested() const;
    bool warnAsErrRequested() const;

    const std::string& getFilesListFile() const;
    const std::string& getFilesListPrefix() const;
    const ArgsList& getFiles() const;
    const std::string& getOutputDirectory() const;
    bool hasNamespaceOverride() const;
    const std::string& getNamespace() const;
    const std::string& getCodeInputDirectory() const;
    bool hasForcedSchemaVersion() const;
    unsigned getForcedSchemaVersion() const;
    const std::string& getProtocolVersion() const;
    unsigned getMinRemoteVersion() const;
    const std::string& getCustomizationLevel() const;
    bool versionIndependentCodeRequested() const;
    std::vector<std::string> getExtraInputBundles() const;
    bool multipleSchemasEnabled() const;
    bool isMainNamespaceInOptionsForced() const;
};

} // namespace commsdsl2comms