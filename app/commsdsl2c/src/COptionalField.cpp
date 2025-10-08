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

#include "COptionalField.h"

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

COptionalField::COptionalField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool COptionalField::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_cExtField = cCast(genExternalField());
    m_cMemField = cCast(genMemberField());
    return true;
}

bool COptionalField::genWriteImpl() const
{
    return cWrite();
}

void COptionalField::cAddHeaderIncludesImpl(CIncludesList& includes) const
{
    if (m_cExtField != nullptr) {
        includes.push_back(m_cExtField->cRelHeader());
    }

    if (m_cMemField != nullptr) {
        m_cMemField->cAddHeaderIncludes(includes);
    }
}

void COptionalField::cAddSourceIncludesImpl(CIncludesList& includes) const
{
    if (m_cMemField != nullptr) {
        m_cMemField->cAddSourceIncludes(includes);
    }
}

void COptionalField::cAddCommsHeaderIncludesImpl(CIncludesList& includes) const
{
    if (m_cExtField != nullptr) {
        includes.push_back(m_cExtField->cRelCommsDefHeader());
    }

    if (m_cMemField != nullptr) {
        m_cMemField->cAddCommsHeaderIncludes(includes);
    }
}


std::string COptionalField::cHeaderCodeImpl() const
{
    static const std::string Templ =
        "#^#MEMBER#$#\n"
        "/// @brief Obtain access to inner non-optional field of @ref #^#NAME#$##^#SUFFIX#$#.\n"
        "#^#FIELD#$#* #^#NAME#$##^#SUFFIX#$#_field(#^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Check the optional field @ref #^#NAME#$##^#SUFFIX#$# exists.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_doesExist(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Force the optional field @ref #^#NAME#$##^#SUFFIX#$# into existance\n"
        "void #^#NAME#$##^#SUFFIX#$#_setExists(#^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Check the optional field @ref #^#NAME#$##^#SUFFIX#$# is missing.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_isMissing(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Force the optional field @ref #^#NAME#$##^#SUFFIX#$# to be missing\n"
        "void #^#NAME#$##^#SUFFIX#$#_setMissing(#^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Check the optional field @ref #^#NAME#$##^#SUFFIX#$# is tentative.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_isTentative(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Force the optional field @ref #^#NAME#$##^#SUFFIX#$# to be tenative\n"
        "void #^#NAME#$##^#SUFFIX#$#_setTentative(#^#NAME#$##^#SUFFIX#$#* field);\n"        
    ;

    auto* field = m_cExtField;
    if (field == nullptr) {
        field = m_cMemField;
        assert(field != nullptr);
    }

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"FIELD", field->cName()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (m_cMemField != nullptr) {
        repl["MEMBER"] = m_cMemField->cHeaderCode();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string COptionalField::cSourceCodeImpl() const
{
    static const std::string Templ =
        "#^#MEMBER#$#\n"
        "#^#FIELD#$#* #^#NAME#$##^#SUFFIX#$#_field(#^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return to#^#FIELD_CONV_SUFFIX#$#(&(from#^#CONV_SUFFIX#$#(field)->field()));\n"
        "}\n"
        "\n"
        "bool #^#NAME#$##^#SUFFIX#$#_doesExist(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->doesExist();\n"
        "}\n"
        "\n"
        "void #^#NAME#$##^#SUFFIX#$#_setExists(#^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->setExists();\n"
        "}\n"
        "\n"
        "bool #^#NAME#$##^#SUFFIX#$#_isMissing(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->isMissing();\n"
        "}\n"
        "\n"
        "void #^#NAME#$##^#SUFFIX#$#_setMissing(#^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->setMissing();\n"
        "}\n"
        "\n"
        "bool #^#NAME#$##^#SUFFIX#$#_isTenative(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->isTentative();\n"
        "}\n"
        "\n"
        "void #^#NAME#$##^#SUFFIX#$#_setTenative(#^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->setTentative();\n"
        "}\n"        
    ;

    auto* field = m_cExtField;
    if (field == nullptr) {
        field = m_cMemField;
        assert(field != nullptr);
    }

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"FIELD", field->cName()},
        {"CONV_SUFFIX", cConversionSuffix()},
        {"FIELD_CONV_SUFFIX", field->cConversionSuffix()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (m_cMemField != nullptr) {
        repl["MEMBER"] = m_cMemField->cSourceCode();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string COptionalField::cCommsHeaderCodeImpl() const
{
    if (m_cMemField != nullptr) {
        return m_cMemField->cCommsHeaderCode();
    }

    return strings::genEmptyString();
}

} // namespace commsdsl2c
