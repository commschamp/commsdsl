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

#include "CommsSchema.h"

#include "CommsGenerator.h"
#include "CommsNamespace.h"

#include <algorithm>

namespace commsdsl2comms
{

CommsSchema::CommsSchema(CommsGenerator& generator, ParseSchema parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

CommsSchema::~CommsSchema() = default;

bool CommsSchema::commsHasAnyMessage() const
{
    return !genGetAllMessages().empty();
}

bool CommsSchema::commsHasReferencedMsgId() const
{
    auto allNs = genGetAllNamespaces();
    return
        std::any_of(
            allNs.begin(), allNs.end(),
            [](auto* ns)
            {
                return CommsNamespace::commsCast(ns)->commsHasReferencedMsgId();
            });
}

bool CommsSchema::commsHasAnyField() const
{
    auto allNs = genGetAllNamespaces();
    return
        std::any_of(
            allNs.begin(), allNs.end(),
            [](auto* ns)
            {
                return CommsNamespace::commsCast(ns)->commsHasAnyField();
            });
}

bool CommsSchema::commsHasAnyGeneratedCode() const
{
    auto allNs = genGetAllNamespaces();
    return
        std::any_of(
            allNs.begin(), allNs.end(),
            [](auto* ns)
            {
                return CommsNamespace::commsCast(ns)->commsHasAnyGeneratedCode();
            });
}

const CommsField* CommsSchema::commsFindValidInterfaceReferencedField(const std::string& refStr) const
{
    for (auto& nsPtr : genNamespaces()) {
        auto* field = CommsNamespace::commsCast(nsPtr.get())->commsFindValidInterfaceReferencedField(refStr);
        if (field != nullptr) {
            return field;
        }
    }

    return nullptr;
}

} // namespace commsdsl2comms
