//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <cassert>
#include <vector>

namespace commsdsl2comms
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
const std::string DebugStr("debug");
const std::string FullDebugStr("d," + DebugStr);
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
const std::string MultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string FullMultipleSchemasEnabledStr("s," + MultipleSchemasEnabledStr);


} // namespace

CommsProgramOptions::CommsProgramOptions()
{
    addHelpOption()
    (VersionStr, "Print version string and exit.")
    (FullQuietStr, "Quiet, show only warnings and errors.")
    (FullDebugStr, "Show debug logging.")
    (FullOutputDirStr, "Output directory path. When not provided current is used.", true)        
    (FullInputFilesListStr, "File containing list of input files.", true)        
    (FullInputFilesPrefixStr, "Prefix for the values from the list file.", true)
    (FullNamespaceStr, "Force protocol namespace. Defaults to schema name.", true) 
    (WarnAsErrStr, "Treat warning as error.")
    (FullCodeInputDirStr, 
        "Directory with code updates.", true)
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
        "\'Name@ListFile\'. The external \'ListFile\' needs to contain a new line separated list of message names "
        "as defined in the CommsDSL. In case the message resides in a namespace its name must be "
        "specified in the same way as being referenced in CommsDSL (\'Namespace.MessageName\'). "
        "The Name part (with separating @) can be omitted, in such case file basename is used as bundle name. "
        "Multiple bundles are separated by comma (\'Name1@ListFile1,Name2@ListFile2\').",
        true)
    (FullMultipleSchemasEnabledStr, 
        "Allow having multiple schemas with different names.")
    ;
}

bool CommsProgramOptions::quietRequested() const
{
    return isOptUsed(QuietStr);
}

bool CommsProgramOptions::debugRequested() const
{
    return isOptUsed(DebugStr);
}

bool CommsProgramOptions::versionRequested() const
{
    return isOptUsed(VersionStr);
}

bool CommsProgramOptions::warnAsErrRequested() const
{
    return isOptUsed(WarnAsErrStr);
}

const std::string& CommsProgramOptions::getFilesListFile() const
{
    return value(InputFilesListStr);
}

const std::string& CommsProgramOptions::getFilesListPrefix() const
{
    return value(InputFilesPrefixStr);
}

const CommsProgramOptions::ArgsList& CommsProgramOptions::getFiles() const
{
    return args();
}

const std::string& CommsProgramOptions::getOutputDirectory() const
{
    return value(OutputDirStr);
}

bool CommsProgramOptions::hasNamespaceOverride() const
{
    return isOptUsed(NamespaceStr);
}

const std::string& CommsProgramOptions::getNamespace() const
{
    return value(NamespaceStr);
}

const std::string& CommsProgramOptions::getCodeInputDirectory() const
{
    return value(CodeInputDirStr);
}

bool CommsProgramOptions::hasForcedSchemaVersion() const
{
    return isOptUsed(ForceVerStr);
}

unsigned CommsProgramOptions::getForcedSchemaVersion() const
{
    return commsdsl::gen::util::strToUnsigned(value(ForceVerStr));
}

const std::string& CommsProgramOptions::getProtocolVersion() const
{
    return value(ProtocolVerStr);
}

unsigned CommsProgramOptions::getMinRemoteVersion() const
{
    if (!isOptUsed(MinRemoteVerStr)) {
        return 0U;
    }

    return commsdsl::gen::util::strToUnsigned(value(MinRemoteVerStr));
}

const std::string& CommsProgramOptions::getCustomizationLevel() const
{
    return value(CustomizationStr);
}

const std::string& CommsProgramOptions::getCommsLibTag() const
{
    return value(CommsTagStr);
}

bool CommsProgramOptions::versionIndependentCodeRequested() const
{
    return isOptUsed(VersionIndependentCodeStr);
}

std::vector<std::string> CommsProgramOptions::getExtraInputBundles() const
{
    return commsdsl::gen::util::strSplitByAnyChar(value(ExtraMessagesBundleStr), ",");    
}

bool CommsProgramOptions::multipleSchemasEnabled() const
{
    return isOptUsed(MultipleSchemasEnabledStr);
}

} // namespace commsdsl2comms
