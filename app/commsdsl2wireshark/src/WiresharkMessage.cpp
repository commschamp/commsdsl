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

#include "WiresharkMessage.h"

#include "WiresharkGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkMessage::WiresharkMessage(WiresharkGenerator& generator, ParseMessage parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

WiresharkMessage::~WiresharkMessage() = default;

std::string WiresharkMessage::wiresharkDissectName() const
{
    auto& wiresharkGenerator = WiresharkGenerator::wiresharkCast(genGenerator());
    return wiresharkGenerator.wiresharkDissectNameFor(*this);
}

std::string WiresharkMessage::wiresharkDissectCode() const
{
    if (!genIsReferenced()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "#^#FIELDS#$#\n"
        "#^#CODE#$#\n"
        ;

    util::GenStringsList fields;
    for (auto* fPtr : m_wiresharkFields) {
        auto str = fPtr->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        fields.push_back(std::move(str));
    }

    // TODO
    util::GenReplacementMap repl = {
        {"FIELDS", util::genStrListToString(fields, "\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool WiresharkMessage::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_wiresharkFields = WiresharkField::wiresharkTransformFieldsList(genFields());
    return true;
}

} // namespace commsdsl2wireshark
