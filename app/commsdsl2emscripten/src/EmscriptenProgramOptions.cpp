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
const std::string EmscriptenHasCodeVerStr("has-code-version");

} // namespace

EmscriptenProgramOptions::EmscriptenProgramOptions()
{
    genAddCommonOptions();
    genAddCodeVersionOptions();
    genAddMessagesSelectionOptions();
    genAddInterfaceSelectionOptions()
    (EmscriptenForceMainNamespaceInNamesStr, "Force having main namespace in generated class names.")
    (EmscriptenHasCodeVerStr, "The protocol definition (produced by commsdsl2comms) contains code semantic version.")
    ;
}

bool EmscriptenProgramOptions::emscriptenIsMainNamespaceInNamesForced() const
{
    return genIsOptUsed(EmscriptenForceMainNamespaceInNamesStr);
}

bool EmscriptenProgramOptions::emscriptenHasCodeVersion() const
{
    return genIsOptUsed(EmscriptenHasCodeVerStr);
}

} // namespace commsdsl2emscripten
