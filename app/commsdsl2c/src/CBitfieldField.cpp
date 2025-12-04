//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CBitfieldField.h"

#include "CGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2c
{

namespace
{

const std::string& cCodeTemplInternal()
{
    static const std::string Templ =
        "#^#MEMBERS#$#\n"
        "#^#ACCESS#$#\n"
        ;

    return Templ;
}

} // namespace

CBitfieldField::CBitfieldField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CBitfieldField::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_cMembers = cTransformFieldsList(genMembers());
    return true;
}

bool CBitfieldField::genWriteImpl() const
{
    return cWrite();
}

void CBitfieldField::cAddHeaderIncludesImpl(CIncludesList& includes) const
{
    for (auto* m : m_cMembers) {
        m->cAddHeaderIncludes(includes);
    }
}

void CBitfieldField::cAddSourceIncludesImpl(CIncludesList& includes) const
{
    for (auto* m : m_cMembers) {
        m->cAddSourceIncludes(includes);
    }
}

void CBitfieldField::cAddCommsHeaderIncludesImpl(CIncludesList& includes) const
{
    for (auto* m : m_cMembers) {
        m->cAddCommsHeaderIncludes(includes);
    }
}

std::string CBitfieldField::cHeaderCodeImpl() const
{
    GenStringsList members;
    GenStringsList access;

    for (auto* m : m_cMembers) {
        members.push_back(m->cHeaderCode());

        static const std::string AccTempl =
            "/// @brief Acquire access to member @ref #^#MEM#$#.\n"
            "#^#MEM#$#* #^#NAME#$##^#SUFFIX#$#_field_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field);\n"
        ;

        util::GenReplacementMap accRepl = {
            {"NAME", cName()},
            {"MEM", m->cName()},
            {"MEM_NAME", comms::genAccessName(m->cGenField().genName())},
        };

        if (cIsVersionOptional()) {
            accRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
        }

        access.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    util::GenReplacementMap repl = {
        {"MEMBERS", util::genStrListToString(members, "\n", "\n")},
        {"ACCESS", util::genStrListToString(access, "\n", "\n")},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CBitfieldField::cSourceCodeImpl() const
{
    GenStringsList members;
    GenStringsList access;

    for (auto* m : m_cMembers) {
        members.push_back(m->cSourceCode());

        static const std::string AccTempl =
            "#^#MEM#$#* #^#NAME#$##^#SUFFIX#$#_field_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field)\n"
            "{\n"
            "    return to#^#MEM_CONV_SUFFIX#$#(&(from#^#CONV_SUFFIX#$#(field)->field_#^#MEM_NAME#$#()));\n"
            "}\n"
        ;

        util::GenReplacementMap accRepl = {
            {"NAME", cName()},
            {"MEM", m->cName()},
            {"MEM_NAME", comms::genAccessName(m->cGenField().genName())},
            {"CONV_SUFFIX", cConversionSuffix()},
            {"MEM_CONV_SUFFIX", m->cConversionSuffix()},
        };

        if (cIsVersionOptional()) {
            accRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
        }

        access.push_back(util::genProcessTemplate(AccTempl, accRepl));
    }

    util::GenReplacementMap repl = {
        {"MEMBERS", util::genStrListToString(members, "\n", "\n")},
        {"ACCESS", util::genStrListToString(access, "\n", "\n")},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CBitfieldField::cCommsHeaderCodeImpl() const
{
    GenStringsList members;
    for (auto* m : m_cMembers) {
        members.push_back(m->cCommsHeaderCode());
    }
    return util::genStrListToString(members, "\n", "\n");
}

} // namespace commsdsl2c
