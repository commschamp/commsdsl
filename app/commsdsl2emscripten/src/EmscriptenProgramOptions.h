//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2emscripten
{

class EmscriptenProgramOptions : public commsdsl::gen::GenProgramOptions
{
public:
    EmscriptenProgramOptions();

    bool quietRequested() const;
    bool versionRequested() const;
    bool warnAsErrRequested() const;

    const std::string& getFilesListFile() const;
    const std::string& getFilesListPrefix() const;
    const ArgsList& getFiles() const;
    const std::string& getOutputDirectory() const;
    const std::string& getCodeInputDirectory() const;
    bool hasNamespaceOverride() const;
    const std::string& getNamespace() const;
    bool multipleSchemasEnabled() const;
    unsigned genGetMinRemoteVersion() const;
    bool isMainNamespaceInNamesForced() const;
    bool hasForcedInterface() const;
    const std::string& getForcedInterface() const;
    bool hasProtocolVersion() const;
    const std::string& messagesListFile() const;
    const std::string& forcedPlatform() const;
};

} // namespace commsdsl2emscripten