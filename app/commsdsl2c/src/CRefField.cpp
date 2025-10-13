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

#include "CRefField.h"

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

CRefField::CRefField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CRefField::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_cRefField = cCast(genReferencedField());
    assert(m_cRefField != nullptr);
    return true;
}

bool CRefField::genWriteImpl() const
{
    return cWrite();
}

void CRefField::cAddHeaderIncludesImpl(CIncludesList& includes) const
{
    includes.push_back(m_cRefField->cRelHeader());
}

void CRefField::cAddSourceIncludesImpl(CIncludesList& includes) const
{
    includes.push_back(m_cRefField->cRelCommsDefHeader());
}

std::string CRefField::cHeaderCodeImpl() const
{
    static const std::string Templ =
        "/// @brief Acquire access to the field referenced by the @ref #^#NAME#$##^#SUFFIX#$#.\n"
        "#^#REF_NAME#$#* #^#NAME#$##^#SUFFIX#$#_ref(#^#NAME#$##^#SUFFIX#$#* field);\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"REF_NAME", m_cRefField->cName()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CRefField::cSourceCodeImpl() const
{
    static const std::string Templ =
        "#^#REF_NAME#$#* #^#NAME#$##^#SUFFIX#$#_ref(#^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return to#^#REF_CONV_SUFFIX#$#(reinterpret_cast<#^#REF_COMMS_TYPE#$#*>(from#^#CONV_SUFFIX#$#(field)));\n"
        "}\n"
    ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"REF_NAME", m_cRefField->cName()},
        {"CONV_SUFFIX", cConversionSuffix()},
        {"REF_CONV_SUFFIX", m_cRefField->cConversionSuffix()},
        {"REF_COMMS_TYPE", m_cRefField->cCommsTypeName()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
