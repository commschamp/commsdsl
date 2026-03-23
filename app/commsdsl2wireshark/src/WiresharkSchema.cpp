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

#include "WiresharkSchema.h"

#include "WiresharkGenerator.h"
#include "WiresharkNamespace.h"

#include "commsdsl/gen/util.h"

namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkSchema::WiresharkSchema(WiresharkGenerator& generator, ParseSchema parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

WiresharkSchema::~WiresharkSchema() = default;

std::string WiresharkSchema::wiresharkDissectCode() const
{
    util::GenStringsList elems;
    for (auto& nsPtr : genNamespaces()) {
        auto str = WiresharkNamespace::wiresharkCast(nsPtr.get())->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "\n", "");
}

std::string WiresharkSchema::wiresharkExtractorsRegCode() const
{
    util::GenStringsList elems;
    for (auto& nsPtr : genNamespaces()) {
        auto str = WiresharkNamespace::wiresharkCast(nsPtr.get())->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "\n", "");
}

} // namespace commsdsl2wireshark
