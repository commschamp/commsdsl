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

#include "commsdsl/gen/GenProgramOptions.h"

#include <iosfwd>
#include <string>
#include <vector>


namespace commsdsl2emscripten
{

class EmscriptenProgramOptions : public commsdsl::gen::GenProgramOptions
{
public:
    EmscriptenProgramOptions();

    bool emscriptenQuietRequested() const;
    bool emscriptenVersionRequested() const;
    bool emscriptenWarnAsErrRequested() const;

    const std::string& emscriptenGetFilesListFile() const;
    const std::string& emscriptenGetFilesListPrefix() const;
    const GenArgsList& emscriptenGetFiles() const;
    const std::string& emscriptenGetOutputDirectory() const;
    const std::string& emscriptenGetCodeInputDirectory() const;
    bool emscriptenHasNamespaceOverride() const;
    const std::string& emscriptenGetNamespace() const;
    bool emscriptenMultipleSchemasEnabled() const;
    unsigned emscriptenGetMinRemoteVersion() const;
    bool emscriptenIsMainNamespaceInNamesForced() const;
    bool emscriptenHasForcedInterface() const;
    const std::string& emscriptenGetForcedInterface() const;
    bool emscriptenHasProtocolVersion() const;
    const std::string& emscriptenMessagesListFile() const;
    const std::string& emscriptenForcedPlatform() const;
};

} // namespace commsdsl2emscripten