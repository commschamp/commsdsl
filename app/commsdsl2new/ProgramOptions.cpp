//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "ProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <iostream>
#include <cassert>
#include <vector>

namespace commsdsl2new
{

namespace
{

#ifdef COMMS_TAG
#define COMMS_TAG_QUITE_(x_) #x_
#define COMMS_TAG_QUITE(x_) COMMS_TAG_QUITE_(x_)
const std::string DefaultCommsTag(COMMS_TAG_QUITE(COMMS_TAG));
#else
const std::string DefaultCommsTag("master");
#endif    

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
const std::string InputFileStr("input-file");
const std::string WarnAsErrStr("warn-as-err");
const std::string CodeInputDirStr("code-input-dir");
const std::string FullCodeInputDirStr("c," + CodeInputDirStr);
const std::string ForceVerStr("force-schema-version");
const std::string ProtocolVerStr("protocol-version");
const std::string FullProtocolVerStr("V," + ProtocolVerStr);
const std::string MinRemoteVerStr("min-remote-version");
const std::string FullMinRemoteVerStr("m," + MinRemoteVerStr);
const std::string CustomizationStr("customization");
const std::string CommsTagStr("comms-tag");
const std::string VersionIndependentCodeStr("version-independent-code");
const std::string ExtraMessagesBundleStr("extra-messages-bundle");

} // namespace

ProgramOptions::ProgramOptions()
{
    addHelpOption()
    (VersionStr, "Print version string and exit.")
    (FullQuietStr, "Quiet, show only warnings and errors.")
    (FullOutputDirStr, "Output directory path. When not provided current is used.", true)        
    (FullInputFilesListStr, "File containing list of input files.", true)        
    (FullInputFilesPrefixStr, "Prefix for the values from the list file.", true)
    (FullNamespaceStr, "Force protocol namespace. Defaults to schema name.", true) 
    (WarnAsErrStr, "Treat warning as error.")
    (FullCodeInputDirStr, 
        "Directories (comma separated) with code updates. Later one takes priority.", true)
    (ForceVerStr, 
        "Force schema version. Must not be greater than version specified in schema file.", true)
    (FullProtocolVerStr, 
        "Specify semantic version of the generated protocol code using <major>.<minor>.<patch> "
        "format to make this information available in the generated code", true)
    (FullMinRemoteVerStr, "Set minimal supported remote version. Defaults to 0.", true)
    (CustomizationStr, 
        "Allowed customization level of generated code. Supported values are:\n"
        "  * \"full\" - For full customization of all fields and messages.\n"
        "  * \"limited\" - For limited customization of variable length fields and messages.\n"
        "  * \"none\" - No compile time customization is allowed.",
        std::string("limited"))
    (CommsTagStr, 
        "Default tag/branch of the COMMS library project, will be used by the "
        "main \"CMakeLists.txt\" file of the generated project.",
        DefaultCommsTag)
    (VersionIndependentCodeStr,
        "By default the generated code is version dependent if at least one defined "
        "interface has \"version\" field. Use this switch to forcefully disable generation "
        "of version denendent code.")
    (ExtraMessagesBundleStr, 
        "Provide extra custom bundle(s) of messages, the relevant code will be added to generated "
        "\"input\" and \"dispatch\" protocol definition folders. The format of the parameter is "
        "\'Name:ListFile\'. The external \'ListFile\' needs to contain a new line separated list of message names "
        "as defined in the CommsDSL. In case the message resides in a namespace its name must be "
        "specified in the same way as being referenced in CommsDSL (\'Namespace.MessageName\'). "
        "Multiple bundles are separated by comma (\'Name1:ListFile1,Name2:ListFile2\').",
        true)
 
    ;
}

bool ProgramOptions::quietRequested() const
{
    return isOptUsed(QuietStr);
}

bool ProgramOptions::versionRequested() const
{
    return isOptUsed(VersionStr);
}

bool ProgramOptions::warnAsErrRequested() const
{
    return isOptUsed(WarnAsErrStr);
}

const std::string& ProgramOptions::getFilesListFile() const
{
    return value(InputFilesListStr);
}

const std::string& ProgramOptions::getFilesListPrefix() const
{
    return value(InputFilesPrefixStr);
}

const ProgramOptions::ArgsList& ProgramOptions::getFiles() const
{
    return args();
}

const std::string& ProgramOptions::getOutputDirectory() const
{
    return value(OutputDirStr);
}

bool ProgramOptions::hasNamespaceOverride() const
{
    return isOptUsed(NamespaceStr);
}

const std::string& ProgramOptions::getNamespace() const
{
    return value(NamespaceStr);
}

std::vector<std::string> ProgramOptions::getCodeInputDirectories() const
{
    return commsdsl::gen::util::strSplitByAnyCharCompressed(value(CodeInputDirStr), ",");
}

bool ProgramOptions::hasForcedSchemaVersion() const
{
    return isOptUsed(ForceVerStr);
}

unsigned ProgramOptions::getForcedSchemaVersion() const
{
    return commsdsl::gen::util::strToUnsigned(value(ForceVerStr));
}

const std::string& ProgramOptions::getProtocolVersion() const
{
    return value(ProtocolVerStr);
}

unsigned ProgramOptions::getMinRemoteVersion() const
{
    if (!isOptUsed(MinRemoteVerStr)) {
        return 0U;
    }

    return commsdsl::gen::util::strToUnsigned(value(MinRemoteVerStr));
}

const std::string& ProgramOptions::getCustomizationLevel() const
{
    return value(CustomizationStr);
}

const std::string& ProgramOptions::getCommsLibTag() const
{
    return value(CommsTagStr);
}

bool ProgramOptions::versionIndependentCodeRequested() const
{
    return isOptUsed(VersionIndependentCodeStr);
}

std::vector<std::string> ProgramOptions::getExtraInputBundles() const
{
    return commsdsl::gen::util::strSplitByAnyCharCompressed(value(ExtraMessagesBundleStr), ",");    
}

} // namespace commsdsl2new
