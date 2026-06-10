//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "WiresharkProgramOptions.h"

#include "commsdsl/gen/util.h"

namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

namespace
{

const std::string WiresharkDefaultPortStr("default-port");
const unsigned WiresharkDefaultPort = 12345;

} // namespace

WiresharkProgramOptions::WiresharkProgramOptions()
{
    genAddCommonOptions();
    genAddMessagesSelectionOptions();
    genAddInterfaceSelectionOptions()
    (WiresharkDefaultPortStr,
        "Default network port. Defaults to " + std::to_string(WiresharkDefaultPort) + '.',
        true)
    ;
}

unsigned WiresharkProgramOptions::wiresharkDefaultPort() const
{
    if (!genIsOptUsed(WiresharkDefaultPortStr)) {
        return WiresharkDefaultPort;
    }

    return util::genStrToUnsigned(genValue(WiresharkDefaultPortStr));
}

} // namespace commsdsl2wireshark
