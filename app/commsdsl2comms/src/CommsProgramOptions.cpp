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

#include "CommsProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <cassert>
#include <vector>

namespace commsdsl2comms
{

namespace
{

const std::string CommsQuietStr("quiet");
const std::string CommsFullQuietStr("q," + CommsQuietStr);
const std::string CommsDebugStr("debug");
const std::string CommsFullDebugStr("d," + CommsDebugStr);
const std::string CommsVersionStr("version");
const std::string CommsOutputDirStr("output-dir");
const std::string CommsFullOutputDirStr("o," + CommsOutputDirStr);
const std::string CommsInputFilesListStr("input-files-list");
const std::string CommsFullInputFilesListStr("i," + CommsInputFilesListStr);
const std::string CommsInputFilesPrefixStr("input-files-prefix");
const std::string CommsFullInputFilesPrefixStr("p," + CommsInputFilesPrefixStr);
const std::string CommsNamespaceStr("namespace");
const std::string CommsFullNamespaceStr("n," + CommsNamespaceStr);
const std::string CommsWarnAsErrStr("warn-as-err");
const std::string CommsCodeInputDirStr("code-input-dir");
const std::string CommsFullCodeInputDirStr("c," + CommsCodeInputDirStr);
const std::string CommsForceVerStr("force-schema-version");
const std::string CommsProtocolVerStr("protocol-version");
const std::string CommsFullProtocolVerStr("V," + CommsProtocolVerStr);
const std::string CommsMinRemoteVerStr("min-remote-version");
const std::string CommsFullMinRemoteVerStr("m," + CommsMinRemoteVerStr);
const std::string CommsCustomizationStr("customization");
const std::string CommsVersionIndependentCodeStr("version-independent-code");
const std::string CommsExtraMessagesBundleStr("extra-messages-bundle");
const std::string CommsMultipleSchemasEnabledStr("multiple-schemas-enabled");
const std::string CommsFullMultipleSchemasEnabledStr("s," + CommsMultipleSchemasEnabledStr);
const std::string CommsForceMainNamespaceInOptionsStr("force-main-ns-in-options");


} // namespace

CommsProgramOptions::CommsProgramOptions()
{
    genAddHelpOption()
    (CommsVersionStr, "Print version string and exit.")
    (CommsFullQuietStr, "Quiet, show only warnings and errors.")
    (CommsFullDebugStr, "Show debug logging.")
    (CommsFullOutputDirStr, "Output directory path. When not provided current is used.", true)        
    (CommsFullInputFilesListStr, "File containing list of input files.", true)        
    (CommsFullInputFilesPrefixStr, "Prefix for the values from the list file.", true)
    (CommsFullNamespaceStr, 
        "Force main namespace change. Defaults to schema name. "
        "In case of having multiple schemas the renaming happends to the last protocol one. "
        "Renaming of non-protocol or multiple schemas is allowed using <orig_name>:<new_name> comma separated pairs.",
        true) 
    (CommsWarnAsErrStr, "Treat warning as error.")
    (CommsFullCodeInputDirStr, 
        "Directory with code updates.", true)
    (CommsForceVerStr, 
        "Force schema version. Must not be greater than version specified in schema file.", true)
    (CommsFullProtocolVerStr, 
        "Specify semantic version of the generated protocol code using <major>.<minor>.<patch> "
        "format to make this information available in the generated code", true)
    (CommsFullMinRemoteVerStr, "Set minimal supported remote version. Defaults to 0.", true)
    (CommsCustomizationStr, 
        "Allowed customization level of generated code. Supported values are:\n"
        "  * \"full\" - For full customization of all fields and messages.\n"
        "  * \"limited\" - For limited customization of variable length fields and messages.\n"
        "  * \"none\" - No compile time customization is allowed.",
        std::string("limited"))
    (CommsVersionIndependentCodeStr,
        "By default the generated code is version dependent if at least one defined "
        "interface has \"version\" field. Use this switch to forcefully disable generation "
        "of version denendent code.")
    (CommsExtraMessagesBundleStr, 
        "Provide extra custom bundle(s) of messages, the relevant code will be added to generated "
        "\"input\" and \"dispatch\" protocol definition folders. The format of the parameter is "
        "\'Name@ListFile\'. The external \'ListFile\' needs to contain a new line separated list of message names "
        "as defined in the CommsDSL. In case the message resides in a namespace its name must be "
        "specified in the same way as being referenced in CommsDSL (\'Namespace.MessageName\'). "
        "The Name part (with separating @) can be omitted, in such case file basename is used as bundle name. "
        "Multiple bundles are separated by comma (\'Name1@ListFile1,Name2@ListFile2\').",
        true)
    (CommsFullMultipleSchemasEnabledStr, 
        "Allow having multiple schemas with different names.")
    (CommsForceMainNamespaceInOptionsStr, "Force having main namespace struct in generated options.")
    ;
}

bool CommsProgramOptions::commsQuietRequested() const
{
    return genIsOptUsed(CommsQuietStr);
}

bool CommsProgramOptions::commsDebugRequested() const
{
    return genIsOptUsed(CommsDebugStr);
}

bool CommsProgramOptions::commsVersionRequested() const
{
    return genIsOptUsed(CommsVersionStr);
}

bool CommsProgramOptions::commsWarnAsErrRequested() const
{
    return genIsOptUsed(CommsWarnAsErrStr);
}

const std::string& CommsProgramOptions::commsGetFilesListFile() const
{
    return genValue(CommsInputFilesListStr);
}

const std::string& CommsProgramOptions::commsGetFilesListPrefix() const
{
    return genValue(CommsInputFilesPrefixStr);
}

const CommsProgramOptions::GenArgsList& CommsProgramOptions::commsGetFiles() const
{
    return genArgs();
}

const std::string& CommsProgramOptions::commsGetOutputDirectory() const
{
    return genValue(CommsOutputDirStr);
}

bool CommsProgramOptions::commsHasNamespaceOverride() const
{
    return genIsOptUsed(CommsNamespaceStr);
}

const std::string& CommsProgramOptions::commsGetNamespace() const
{
    return genValue(CommsNamespaceStr);
}

const std::string& CommsProgramOptions::commsGetCodeInputDirectory() const
{
    return genValue(CommsCodeInputDirStr);
}

bool CommsProgramOptions::commsHasForcedSchemaVersion() const
{
    return genIsOptUsed(CommsForceVerStr);
}

unsigned CommsProgramOptions::commsGetForcedSchemaVersion() const
{
    return commsdsl::gen::util::genStrToUnsigned(genValue(CommsForceVerStr));
}

const std::string& CommsProgramOptions::commsGetProtocolVersion() const
{
    return genValue(CommsProtocolVerStr);
}

unsigned CommsProgramOptions::commsGetMinRemoteVersion() const
{
    if (!genIsOptUsed(CommsMinRemoteVerStr)) {
        return 0U;
    }

    return commsdsl::gen::util::genStrToUnsigned(genValue(CommsMinRemoteVerStr));
}

const std::string& CommsProgramOptions::commsGetCustomizationLevel() const
{
    return genValue(CommsCustomizationStr);
}

bool CommsProgramOptions::commsVersionIndependentCodeRequested() const
{
    return genIsOptUsed(CommsVersionIndependentCodeStr);
}

std::vector<std::string> CommsProgramOptions::commsGetExtraInputBundles() const
{
    return commsdsl::gen::util::genStrSplitByAnyChar(genValue(CommsExtraMessagesBundleStr), ",");    
}

bool CommsProgramOptions::commsMultipleSchemasEnabled() const
{
    return genIsOptUsed(CommsMultipleSchemasEnabledStr);
}

bool CommsProgramOptions::commsIsMainNamespaceInOptionsForced() const
{
    return genIsOptUsed(CommsForceMainNamespaceInOptionsStr);
}

} // namespace commsdsl2comms
