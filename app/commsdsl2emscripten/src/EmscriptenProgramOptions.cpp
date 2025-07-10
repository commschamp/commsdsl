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

#include "EmscriptenProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <iostream>
#include <cassert>
#include <vector>

namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

namespace
{

const std::string EmscriptenQuietStr("quiet");
const std::string EmscriptenFullQuietStr("q," + EmscriptenQuietStr);
const std::string EmscriptenVersionStr("version");
const std::string EmscriptenOutputDirStr("output-dir");
const std::string EmscriptenFullOutputDirStr("o," + EmscriptenOutputDirStr);
const std::string EmscriptenInputFilesListStr("input-files-list");
const std::string EmscriptenFullInputFilesListStr("i," + EmscriptenInputFilesListStr);
const std::string EmscriptenInputFilesPrefixStr("input-files-prefix");
const std::string EmscriptenFullInputFilesPrefixStr("p," + EmscriptenInputFilesPrefixStr);
const std::string EmscriptenNamespaceStr("namespace");
const std::string EmscriptenFullNamespaceStr("n," + EmscriptenNamespaceStr);
const std::string EmscriptenWarnAsErrStr("warn-as-err");
const std::string EmscriptenCodeInputDirStr("code-input-dir");
const std::string EmscriptenFullCodeInputDirStr("c," + EmscriptenCodeInputDirStr);
const std::string EmscriptenMultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string EmscriptenFullMultipleSchemasEnabledStr("s," + EmscriptenMultipleSchemasEnabledStr);
const std::string EmscriptenMinRemoteVerStr("min-remote-version");
const std::string EmscriptenFullMinRemoteVerStr("m," + EmscriptenMinRemoteVerStr);
const std::string EmscriptenForceMainNamespaceInNamesStr("force-main-ns-in-names");
const std::string EmscriptenForceInterfaceStr("force-interface");
const std::string EmscriptenHasProtocolStr("has-protocol-version");
const std::string EmscriptenMessagesListStr("messages-list");
const std::string EmscriptenForcePlatformStr("force-platform");

} // namespace

EmscriptenProgramOptions::EmscriptenProgramOptions()
{
    genAddHelpOption()
    (EmscriptenVersionStr, "Print version string and exit.")
    (EmscriptenFullQuietStr.c_str(), "Quiet, show only warnings and errors.")
    (EmscriptenFullOutputDirStr.c_str(), "Output directory path. When not provided current is used.", true)        
    (EmscriptenFullInputFilesListStr.c_str(), "File containing list of input files.", true)        
    (EmscriptenFullInputFilesPrefixStr.c_str(), "Prefix for the values from the list file.", true)
    (EmscriptenFullNamespaceStr, 
        "Force main namespace change. Defaults to schema name. "
        "In case of having multiple schemas the renaming happends to the last protocol one. "
        "Renaming of non-protocol or multiple schemas is allowed using <orig_name>:<new_name> comma separated pairs.",
        true) 
    (EmscriptenWarnAsErrStr.c_str(), "Treat warning as error.")
    (EmscriptenFullCodeInputDirStr, "Directory with code updates.", true)
    (EmscriptenFullMultipleSchemasEnabledStr, "Allow having multiple schemas with different names.")    
    (EmscriptenFullMinRemoteVerStr, "Set minimal supported remote version. Defaults to 0.", true)
    (EmscriptenForceMainNamespaceInNamesStr, "Force having main namespace in generated class names.")
    (EmscriptenForceInterfaceStr, "Force usage of the provided interface (CommsDSL reference string).", true)
    (EmscriptenHasProtocolStr, "The protocol definition (produced by commsdsl2comms) contains protocol semantic version.")
    (EmscriptenMessagesListStr, 
        "Path to the file containing list of messages that need to be supported. "
        "In case the message resides in a namespace its name must be "
        "specified in the same way as being referenced in CommsDSL (\'Namespace.MessageName\'). "
        "If not provided all the defined messages are going to be supported.",
        true)
    (EmscriptenForcePlatformStr, "Support only messages applicable to specified platform. Requires protocol schema to define it.", true)        
    ;
}

bool EmscriptenProgramOptions::emscriptenQuietRequested() const
{
    return genIsOptUsed(EmscriptenQuietStr);
}

bool EmscriptenProgramOptions::emscriptenVersionRequested() const
{
    return genIsOptUsed(EmscriptenVersionStr);
}

bool EmscriptenProgramOptions::emscriptenWarnAsErrRequested() const
{
    return genIsOptUsed(EmscriptenWarnAsErrStr);
}

const std::string& EmscriptenProgramOptions::emscriptenGetFilesListFile() const
{
    return genValue(EmscriptenInputFilesListStr);
}

const std::string& EmscriptenProgramOptions::emscriptenGetFilesListPrefix() const
{
    return genValue(EmscriptenInputFilesPrefixStr);
}

const EmscriptenProgramOptions::GenArgsList& EmscriptenProgramOptions::emscriptenGetFiles() const
{
    return genArgs();
}

const std::string& EmscriptenProgramOptions::emscriptenGetOutputDirectory() const
{
    return genValue(EmscriptenOutputDirStr);
}

const std::string& EmscriptenProgramOptions::emscriptenGetCodeInputDirectory() const
{
    return genValue(EmscriptenCodeInputDirStr);
}

bool EmscriptenProgramOptions::emscriptenHasNamespaceOverride() const
{
    return genIsOptUsed(EmscriptenNamespaceStr);
}

const std::string& EmscriptenProgramOptions::emscriptenGetNamespace() const
{
    return genValue(EmscriptenNamespaceStr);
}

bool EmscriptenProgramOptions::emscriptenMultipleSchemasEnabled() const
{
    return genIsOptUsed(EmscriptenMultipleSchemasEnabledStr);
}

unsigned EmscriptenProgramOptions::emscriptenGetMinRemoteVersion() const
{
    if (!genIsOptUsed(EmscriptenMinRemoteVerStr)) {
        return 0U;
    }

    return util::genStrToUnsigned(genValue(EmscriptenMinRemoteVerStr));
}

bool EmscriptenProgramOptions::emscriptenIsMainNamespaceInNamesForced() const
{
    return genIsOptUsed(EmscriptenForceMainNamespaceInNamesStr);
}

bool EmscriptenProgramOptions::emscriptenHasForcedInterface() const
{
    return genIsOptUsed(EmscriptenForceInterfaceStr);
}

const std::string& EmscriptenProgramOptions::emscriptenGetForcedInterface() const
{
    return genValue(EmscriptenForceInterfaceStr);
}

bool EmscriptenProgramOptions::emscriptenHasProtocolVersion() const
{
    return genIsOptUsed(EmscriptenHasProtocolStr);
}

const std::string& EmscriptenProgramOptions::emscriptenMessagesListFile() const
{
    return genValue(EmscriptenMessagesListStr);
}

const std::string& EmscriptenProgramOptions::emscriptenForcedPlatform() const
{
    return genValue(EmscriptenForcePlatformStr);
}

} // namespace commsdsl2emscripten
