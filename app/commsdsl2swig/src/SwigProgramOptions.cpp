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
const std::string HasCodeVerStr("has-code-version");

} // namespace

SwigProgramOptions::SwigProgramOptions()
{
    genAddCommonOptions();
    genAddCodeVersionOptions();
    genAddMessagesSelectionOptions();
    genAddInterfaceSelectionOptions()
    (ForceMainNamespaceInNamesStr, "Force having main namespace in generated class names.")
    (HasCodeVerStr, "The protocol definition (produced by commsdsl2comms) contains code semantic version.")
    ;
}

bool SwigProgramOptions::swigIsMainNamespaceInNamesForced() const
{
    return genIsOptUsed(ForceMainNamespaceInNamesStr);
}

bool SwigProgramOptions::swigHasCodeVersion() const
{
    return genIsOptUsed(HasCodeVerStr);
}

} // namespace commsdsl2swig
