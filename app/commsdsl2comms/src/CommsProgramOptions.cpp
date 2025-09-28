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

const std::string CommsProtocolVerStr("protocol-version");
const std::string CommsFullProtocolVerStr("V," + CommsProtocolVerStr);
const std::string CommsCustomizationStr("customization");
const std::string CommsVersionIndependentCodeStr("version-independent-code");
const std::string CommsExtraMessagesBundleStr("extra-messages-bundle");
const std::string CommsForceMainNamespaceInOptionsStr("force-main-ns-in-options");

} // namespace

CommsProgramOptions::CommsProgramOptions()
{
    genAddCommonOptions()
    (CommsFullProtocolVerStr,
        "Specify semantic version of the generated protocol code using <major>.<minor>.<patch> "
        "format to make this information available in the generated code", true)
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
    (CommsForceMainNamespaceInOptionsStr, "Force having main namespace struct in generated options.")
    ;
}

const std::string& CommsProgramOptions::commsGetProtocolVersion() const
{
    return genValue(CommsProtocolVerStr);
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

bool CommsProgramOptions::commsIsMainNamespaceInOptionsForced() const
{
    return genIsOptUsed(CommsForceMainNamespaceInOptionsStr);
}

} // namespace commsdsl2comms
