//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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
    [[nodiscard]] bool genIsOptUsed(const std::string& optStr) const;
    [[nodiscard]] const std::string& genValue(const std::string& optStr) const;
    [[nodiscard]] const GenStringsList& genArgs() const;
    [[nodiscard]] const std::string& genApp() const;
    [[nodiscard]] std::string genHelpStr() const;

    [[nodiscard]] bool genHelpRequested() const;
    [[nodiscard]] bool genVersionRequested() const;
    [[nodiscard]] bool genQuietRequested() const;
    [[nodiscard]] bool genDebugRequested() const;
    [[nodiscard]] bool genWarnAsErrRequested() const;
    [[nodiscard]] const std::string& genGetOutputDirectory() const;
    [[nodiscard]] GenStringsList genGetInputFiles() const;
    [[nodiscard]] const std::string& genGetCodeInputDirectory() const;
    [[nodiscard]] bool genMultipleSchemasEnabled() const;
    [[nodiscard]] bool genHasNamespaceOverride() const;
    [[nodiscard]] const std::string& genGetNamespace() const;
    [[nodiscard]] unsigned genGetMinRemoteVersion() const;
    [[nodiscard]] bool genHasForcedSchemaVersion() const;
    [[nodiscard]] unsigned genGetForcedSchemaVersion() const;
    [[nodiscard]] bool genCodeInjectComments() const;
    [[nodiscard]] const std::string& genGetCodeVersion() const;
    [[nodiscard]] const std::string& genMessagesListFile() const;
    [[nodiscard]] const std::string& genForcedPlatform() const;
    [[nodiscard]] const std::string& genForcedInterface() const;

private:
    std::unique_ptr<GenProgramOptionsImpl> m_impl;
};

} // namespace gen

} // namespace commsdsl
