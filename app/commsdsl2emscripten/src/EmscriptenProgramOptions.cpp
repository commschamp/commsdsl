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

const std::string QuietStr("quiet");
const std::string FullQuietStr("q," + QuietStr);
const std::string VersionStr("version");
const std::string OutputDirStr("output-dir");
const std::string FullOutputDirStr("o," + OutputDirStr);
const std::string InputFilesListStr("input-files-list");
const std::string FullInputFilesListStr("i," + InputFilesListStr);
const std::string InputFilesPrefixStr("input-files-prefix");
const std::string FullInputFilesPrefixStr("p," + InputFilesPrefixStr);
const std::string NamespaceStr("namespace");
const std::string FullNamespaceStr("n," + NamespaceStr);
const std::string WarnAsErrStr("warn-as-err");
const std::string CodeInputDirStr("code-input-dir");
const std::string FullCodeInputDirStr("c," + CodeInputDirStr);
const std::string MultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string FullMultipleSchemasEnabledStr("s," + MultipleSchemasEnabledStr);
const std::string MinRemoteVerStr("min-remote-version");
const std::string FullMinRemoteVerStr("m," + MinRemoteVerStr);
const std::string ForceMainNamespaceInNamesStr("force-main-ns-in-names");
const std::string ForceInterfaceStr("force-interface");
const std::string HasProtocolStr("has-protocol-version");
const std::string MessagesListStr("messages-list");
const std::string ForcePlatformStr("force-platform");

} // namespace

EmscriptenProgramOptions::EmscriptenProgramOptions()
{
    addHelpOption()
    (VersionStr, "Print version string and exit.")
    (FullQuietStr.c_str(), "Quiet, show only warnings and errors.")
    (FullOutputDirStr.c_str(), "Output directory path. When not provided current is used.", true)        
    (FullInputFilesListStr.c_str(), "File containing list of input files.", true)        
    (FullInputFilesPrefixStr.c_str(), "Prefix for the values from the list file.", true)
    (FullNamespaceStr, 
        "Force main namespace change. Defaults to schema name. "
        "In case of having multiple schemas the renaming happends to the last protocol one. "
        "Renaming of non-protocol or multiple schemas is allowed using <orig_name>:<new_name> comma separated pairs.",
        true) 
    (WarnAsErrStr.c_str(), "Treat warning as error.")
    (FullCodeInputDirStr, "Directory with code updates.", true)
    (FullMultipleSchemasEnabledStr, "Allow having multiple schemas with different names.")    
    (FullMinRemoteVerStr, "Set minimal supported remote version. Defaults to 0.", true)
    (ForceMainNamespaceInNamesStr, "Force having main namespace in generated class names.")
    (ForceInterfaceStr, "Force usage of the provided interface (CommsDSL reference string).", true)
    (HasProtocolStr, "The protocol definition (produced by commsdsl2comms) contains protocol semantic version.")
    (MessagesListStr, 
        "Path to the file containing list of messages that need to be supported. "
        "In case the message resides in a namespace its name must be "
        "specified in the same way as being referenced in CommsDSL (\'Namespace.MessageName\'). "
        "If not provided all the defined messages are going to be supported.",
        true)
    (ForcePlatformStr, "Support only messages applicable to specified platform. Requires protocol schema to define it.", true)        
    ;
}

bool EmscriptenProgramOptions::quietRequested() const
{
    return isOptUsed(QuietStr);
}

bool EmscriptenProgramOptions::versionRequested() const
{
    return isOptUsed(VersionStr);
}

bool EmscriptenProgramOptions::warnAsErrRequested() const
{
    return isOptUsed(WarnAsErrStr);
}

const std::string& EmscriptenProgramOptions::getFilesListFile() const
{
    return value(InputFilesListStr);
}

const std::string& EmscriptenProgramOptions::getFilesListPrefix() const
{
    return value(InputFilesPrefixStr);
}

const EmscriptenProgramOptions::ArgsList& EmscriptenProgramOptions::getFiles() const
{
    return args();
}

const std::string& EmscriptenProgramOptions::getOutputDirectory() const
{
    return value(OutputDirStr);
}

const std::string& EmscriptenProgramOptions::getCodeInputDirectory() const
{
    return value(CodeInputDirStr);
}

bool EmscriptenProgramOptions::hasNamespaceOverride() const
{
    return isOptUsed(NamespaceStr);
}

const std::string& EmscriptenProgramOptions::getNamespace() const
{
    return value(NamespaceStr);
}

bool EmscriptenProgramOptions::multipleSchemasEnabled() const
{
    return isOptUsed(MultipleSchemasEnabledStr);
}

unsigned EmscriptenProgramOptions::getMinRemoteVersion() const
{
    if (!isOptUsed(MinRemoteVerStr)) {
        return 0U;
    }

    return util::strToUnsigned(value(MinRemoteVerStr));
}

bool EmscriptenProgramOptions::isMainNamespaceInNamesForced() const
{
    return isOptUsed(ForceMainNamespaceInNamesStr);
}

bool EmscriptenProgramOptions::hasForcedInterface() const
{
    return isOptUsed(ForceInterfaceStr);
}

const std::string& EmscriptenProgramOptions::getForcedInterface() const
{
    return value(ForceInterfaceStr);
}

bool EmscriptenProgramOptions::hasProtocolVersion() const
{
    return isOptUsed(HasProtocolStr);
}

const std::string& EmscriptenProgramOptions::messagesListFile() const
{
    return value(MessagesListStr);
}

const std::string& EmscriptenProgramOptions::forcedPlatform() const
{
    return value(ForcePlatformStr);
}

} // namespace commsdsl2emscripten
