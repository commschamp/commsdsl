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

#include "SwigSchema.h"

#include "SwigGenerator.h"
#include "SwigNamespace.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

SwigSchema::SwigSchema(SwigGenerator& generator, ParseSchema parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

SwigSchema::~SwigSchema() = default;

bool SwigSchema::swigHasAnyMessage() const
{
    return genHasAnyReferencedMessage();
}

bool SwigSchema::swigHasReferencedMsgId() const
{
    return genHasReferencedMessageIdField();
}

void SwigSchema::swigAddCodeIncludes(GenStringsList& list) const
{
    for (auto& ns : genNamespaces()) {
        SwigNamespace::swigCast(ns.get())->swigAddCodeIncludes(list);
    }
}

void SwigSchema::swigAddCode(GenStringsList& list) const
{
    for (auto& ns : genNamespaces()) {
        SwigNamespace::swigCast(ns.get())->swigAddCode(list);
    }
}

void SwigSchema::swigAddDef(GenStringsList& list) const
{
    for (auto& ns : genNamespaces()) {
        SwigNamespace::swigCast(ns.get())->swigAddDef(list);
    }
}

} // namespace commsdsl2swig
