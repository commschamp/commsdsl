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

#include "WiresharkVariantField.h"

#include "Wireshark.h"
#include "WiresharkGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2wireshark
{

WiresharkVariantField::WiresharkVariantField(WiresharkGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    WiresharkBase(static_cast<GenBase&>(*this))
{
}

bool WiresharkVariantField::genPrepareImpl()
{
    if ((!GenBase::genPrepareImpl()) ||
        (!WiresharkBase::wiresharkPrepare())) {
        return false;
    }

    m_wiresharkFields = wiresharkTransformFieldsList(genMembers());
    auto& generator = genGenerator();
    m_wiresharkFields.erase(
        std::remove_if(
            m_wiresharkFields.begin(), m_wiresharkFields.end(),
            [&generator](auto* fPtr)
            {
                auto parseObj = fPtr->wiresharkGenField().genParseObj();
                return !generator.genDoesElementExist(parseObj.parseSinceVersion(), parseObj.parseDeprecatedSince(), parseObj.parseIsDeprecatedRemoved());
            }),
        m_wiresharkFields.end());
    return true;
}

std::string WiresharkVariantField::wiresharkFieldRegistrationImpl(const WiresharkField* refField) const
{
    static const std::string Templ =
        "local #^#OBJ_NAME#$# = #^#CREATE_FUNC#$#(ProtoField.bytes(\"#^#REF_NAME#$#\", #^#DISP_NAME#$#, base.SPACE, #^#DESC#$#))\n"
    ;

    auto obj = genParseObj();
    util::GenReplacementMap repl = {
        {"OBJ_NAME", wiresharkFieldObjName(refField)},
        {"CREATE_FUNC", Wireshark::wiresharkCreateFieldFuncName(WiresharkGenerator::wiresharkCast(genGenerator()))},
        {"REF_NAME", wiresharkFieldRefName(refField)},
        {"DISP_NAME", wiresharkFieldNameVarNameStr(refField)},
        {"DESC", wiresharkFieldDescriptionStr(refField)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string WiresharkVariantField::wiresharkMembersDissectCodeImpl() const
{
    util::GenStringsList elems;
    for (auto* f : m_wiresharkFields) {
        auto str = f->wiresharkDissectCode();
        if (str.empty()) {
            continue;
        }

        elems.push_back(std::move(str));
    }

    return util::genStrListToString(elems, "\n", "\n");
}

} // namespace commsdsl2wireshark
