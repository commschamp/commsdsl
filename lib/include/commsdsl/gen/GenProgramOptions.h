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

#include "commsdsl/CommsdslApi.h"

#include <memory>
#include <string>
#include <vector>

namespace commsdsl
{

namespace gen
{

class GenProgramOptionsImpl;
class COMMSDSL_API GenProgramOptions
{
public:
    using GenStringsList = std::vector<std::string>;

    GenProgramOptions();
    ~GenProgramOptions();

    GenProgramOptions& genAddCommonOptions();
    GenProgramOptions& genAddCodeVersionOptions();
    GenProgramOptions& genAddMessagesSelectionOptions();
    GenProgramOptions& genAddInterfaceSelectionOptions();
    GenProgramOptions& genRemoveMinRemoteVersionOptions();
    GenProgramOptions& operator()(const std::string& optStr, const std::string& desc, bool hasParam = false);
    GenProgramOptions& operator()(const std::string& optStr, const std::string& desc, const std::string& defaultValue);

    void genParse(int argc, const char** argv);
    bool genIsOptUsed(const std::string& optStr) const;
    const std::string& genValue(const std::string& optStr) const;
    const GenStringsList& genArgs() const;
    const std::string& genApp() const;
    std::string genHelpStr() const;

    bool genHelpRequested() const;
    bool genVersionRequested() const;
    bool genQuietRequested() const;
    bool genDebugRequested() const;
    bool genWarnAsErrRequested() const;
    const std::string& genGetOutputDirectory() const;
    GenStringsList genGetInputFiles() const;
    const std::string& genGetCodeInputDirectory() const;
    bool genMultipleSchemasEnabled() const;
    bool genHasNamespaceOverride() const;
    const std::string& genGetNamespace() const;
    unsigned genGetMinRemoteVersion() const;
    bool genHasForcedSchemaVersion() const;
    unsigned genGetForcedSchemaVersion() const;
    const std::string& genGetCodeVersion() const;
    const std::string& genMessagesListFile() const;
    const std::string& genForcedPlatform() const;
    const std::string& genForcedInterface() const;

private:
    std::unique_ptr<GenProgramOptionsImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
