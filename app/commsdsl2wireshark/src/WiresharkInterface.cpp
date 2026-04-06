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

#include "WiresharkInterface.h"

#include "WiresharkGenerator.h"
#include "WiresharkNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkInterface::WiresharkInterface(WiresharkGenerator& generator, ParseInterface parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

WiresharkInterface::~WiresharkInterface() = default;

bool WiresharkInterface::wiresharkDissectionAllowed() const
{
    auto* parentNs = WiresharkNamespace::wiresharkCast(genParentNamespace());
    assert(parentNs);
    return this == parentNs->wiresharkInterface();
}

std::string WiresharkInterface::wiresharkDissectCode() const
{
    if ((!genIsReferenced()) || m_wiresharkFields.empty() || (!wiresharkDissectionAllowed())) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "#^#FIELDS#$#\n"
        ;

    util::GenStringsList fields;
    for (auto* fPtr : m_wiresharkFields) {
        auto str = fPtr->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        fields.push_back(std::move(str));
    }

    util::GenReplacementMap repl = {
        {"FIELDS", util::genStrListToString(fields, "\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkInterface::wiresharkExtractorsRegCode() const
{
    if ((!genIsReferenced()) || m_wiresharkFields.empty() || (!wiresharkDissectionAllowed())) {
        return strings::genEmptyString();
    }

    util::GenStringsList fields;
    for (auto* fPtr : m_wiresharkFields) {
        auto str = fPtr->wiresharkExtractorsRegCode();
        if (str.empty()) {
            continue;
        }

        fields.push_back(std::move(str));
    }

    return util::genStrListToString(fields, "", "");
}

bool WiresharkInterface::wiresharkNeedsOptionalModeDefinition() const
{
    if ((!genIsReferenced()) || (!wiresharkDissectionAllowed())) {
        return false;
    }

    return
        std::any_of(
            m_wiresharkFields.begin(), m_wiresharkFields.end(),
            [](auto* fPtr)
            {
                return fPtr->wiresharkNeedsOptionalModeDefinition();
            });
}

const WiresharkInterface::WiresharkFieldsList& WiresharkInterface::wiresharkMemberFields() const
{
    return m_wiresharkFields;
}

std::string WiresharkInterface::wiresharkDefaultAssignments() const
{
    util::GenStringsList elems;
    for (auto* f : m_wiresharkFields) {
        auto str = f->wiresharkDefaultAssignments();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "", "");
}

const WiresharkField* WiresharkInterface::wiresharkFindField(const std::string& name) const
{
    auto iter =
        std::find_if(
            m_wiresharkFields.begin(), m_wiresharkFields.end(),
            [&name](auto* f)
            {
                return f->wiresharkGenField().genParseObj().parseName() == name;
            });

    if (iter == m_wiresharkFields.end()) {
        return nullptr;
    }

    return *iter;
}

bool WiresharkInterface::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_wiresharkFields = WiresharkField::wiresharkTransformFieldsList(genFields());
    return true;
}

} // namespace commsdsl2wireshark
