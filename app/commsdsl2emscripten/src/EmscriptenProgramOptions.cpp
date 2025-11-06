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

const std::string EmscriptenForceMainNamespaceInNamesStr("force-main-ns-in-names");
const std::string EmscriptenForceInterfaceStr("force-interface");
const std::string EmscriptenHasCodeVerStr("has-code-version");
const std::string EmscriptenMessagesListStr("messages-list");
const std::string EmscriptenForcePlatformStr("force-platform");

} // namespace

EmscriptenProgramOptions::EmscriptenProgramOptions()
{
    genAddCommonOptions().genAddCodeVersionOptions()
        (EmscriptenForceMainNamespaceInNamesStr, "Force having main namespace in generated class names.")
        (EmscriptenForceInterfaceStr, "Force usage of the provided interface (CommsDSL reference string).", true)
        (EmscriptenHasCodeVerStr, "The protocol definition (produced by commsdsl2comms) contains code semantic version.")
        (EmscriptenMessagesListStr,
            "Path to the file containing list of messages that need to be supported. "
            "In case the message resides in a namespace its name must be "
            "specified in the same way as being referenced in CommsDSL (\'Namespace.MessageName\'). "
            "If not provided all the defined messages are going to be supported.",
            true)
        (EmscriptenForcePlatformStr, "Support only messages applicable to specified platform. Requires protocol schema to define it.", true)
        ;
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

bool EmscriptenProgramOptions::emscriptenHasCodeVersion() const
{
    return genIsOptUsed(EmscriptenHasCodeVerStr);
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
