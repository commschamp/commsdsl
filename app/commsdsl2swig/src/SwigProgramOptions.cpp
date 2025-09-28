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

#include "SwigProgramOptions.h"

#include "commsdsl/gen/util.h"

#include <iostream>
#include <cassert>
#include <vector>

namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

namespace
{

const std::string ForceMainNamespaceInNamesStr("force-main-ns-in-names");
const std::string ForceInterfaceStr("force-interface");
const std::string HasProtocolStr("has-protocol-version");
const std::string MessagesListStr("messages-list");
const std::string ForcePlatformStr("force-platform");

} // namespace

SwigProgramOptions::SwigProgramOptions()
{
    genAddCommonOptions()
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

bool SwigProgramOptions::swigIsMainNamespaceInNamesForced() const
{
    return genIsOptUsed(ForceMainNamespaceInNamesStr);
}

bool SwigProgramOptions::swigHasForcedInterface() const
{
    return genIsOptUsed(ForceInterfaceStr);
}

const std::string& SwigProgramOptions::swigGetForcedInterface() const
{
    return genValue(ForceInterfaceStr);
}

bool SwigProgramOptions::swigHasProtocolVersion() const
{
    return genIsOptUsed(HasProtocolStr);
}

const std::string& SwigProgramOptions::swigMessagesListFile() const
{
    return genValue(MessagesListStr);
}

const std::string& SwigProgramOptions::swigForcedPlatform() const
{
    return genValue(ForcePlatformStr);
}

} // namespace commsdsl2swig
