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

#include "CListField.h"

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

CListField::CListField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CListField::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    auto* externalElem = genExternalElementField();
    if (externalElem != nullptr) {
        m_cExternalElementField = cCast(externalElem);
    }

    auto* memberElem = genMemberElementField();
    if (memberElem != nullptr) {
        m_cMemberElementField = cCast(memberElem);
    }

    return true;
}

bool CListField::genWriteImpl() const
{
    return cWrite();
}

void CListField::cAddHeaderIncludesImpl(CIncludesList& includes) const
{
    if (m_cExternalElementField != nullptr) {
        includes.push_back(m_cExternalElementField->cRelHeader());
    }

    if (m_cMemberElementField != nullptr) {
        m_cMemberElementField->cAddHeaderIncludes(includes);
    }
}

void CListField::cAddSourceIncludesImpl(CIncludesList& includes) const
{
    includes.push_back("<algorithm>");

    if (m_cMemberElementField != nullptr) {
        m_cMemberElementField->cAddSourceIncludes(includes);
    }
}

void CListField::cAddCommsHeaderIncludesImpl(CIncludesList& includes) const
{
    if (m_cExternalElementField != nullptr) {
        includes.push_back(m_cExternalElementField->cRelCommsDefHeader());
    }

    if (m_cMemberElementField != nullptr) {
        m_cMemberElementField->cAddCommsHeaderIncludes(includes);
    }
}

std::string CListField::cHeaderCodeImpl() const
{
    static const std::string Templ =
        "#^#MEMBER#$#\n"
        "/// @brief Acquire access for the element of @ref #^#NAME#$##^#SUFFIX#$#.\n"
        "/// @return Pointer to the element, @b NULL in case @b idx is out of bounds.\n"
        "#^#ELEMENT#$#* #^#NAME#$##^#SUFFIX#$#_at(#^#NAME#$##^#SUFFIX#$#* field, size_t idx);\n"
        "\n"
        "/// @brief Resize the inner storage of @ref #^#NAME#$##^#SUFFIX#$# field to fit specified amount of elements.\n"
        "void #^#NAME#$##^#SUFFIX#$#_resize(#^#NAME#$##^#SUFFIX#$#* field, size_t count);\n"
        "\n"
        "/// @brief Retrieve amount of currently stored elements in @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_count(const #^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Insert new element of @ref #^#NAME#$##^#SUFFIX#$# at specified position.\n"
        "/// @return Pointer to the inserted element, @b NULL in case inner storage grows out of capacity.\n"
        "#^#ELEMENT#$#* #^#NAME#$##^#SUFFIX#$#_insert(#^#NAME#$##^#SUFFIX#$#* field, size_t idx);\n"
        "\n"
        "/// @brief Append new element of @ref #^#NAME#$##^#SUFFIX#$# at end.\n"
        "/// @return Pointer to the appended element, @b NULL in case inner storage grows out of capacity.\n"
        "#^#ELEMENT#$#* #^#NAME#$##^#SUFFIX#$#_append(#^#NAME#$##^#SUFFIX#$#* field);\n"
        "\n"
        "/// @brief Remove element of @ref #^#NAME#$##^#SUFFIX#$# at specified position.\n"
        "/// @return @b true in case of element removed, and @b false if element doesn't exit.\n"
        "bool #^#NAME#$##^#SUFFIX#$#_remove(#^#NAME#$##^#SUFFIX#$#* field, size_t idx);\n"
    ;

    auto* element = m_cExternalElementField;
    if (element == nullptr) {
        element = m_cMemberElementField;
        assert(element != nullptr);
    }

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"ELEMENT", element->cName()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (m_cMemberElementField != nullptr) {
        repl["MEMBER"] = m_cMemberElementField->cHeaderCode();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CListField::cSourceCodeImpl() const
{
    static const std::string Templ =
        "#^#MEMBER#$#\n"
        "#^#ELEMENT#$#* #^#NAME#$##^#SUFFIX#$#_at(#^#NAME#$##^#SUFFIX#$#* field, size_t idx)\n"
        "{\n"
        "    auto& val = from#^#CONV_SUFFIX#$#(field)->value();\n"
        "    if (val.size() <= idx) {\n"
        "        return nullptr;\n"
        "    }\n"
        "    return to#^#ELEM_CONV_SUFFIX#$#(&val[idx]);\n"
        "}\n"
        "\n"
        "void #^#NAME#$##^#SUFFIX#$#_resize(#^#NAME#$##^#SUFFIX#$#* field, size_t count)\n"
        "{\n"
        "    from#^#CONV_SUFFIX#$#(field)->value().resize(count);\n"
        "}\n"
        "\n"
        "size_t #^#NAME#$##^#SUFFIX#$#_count(const #^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    return from#^#CONV_SUFFIX#$#(field)->value().size();\n"
        "}\n"
        "\n"
        "#^#ELEMENT#$#* #^#NAME#$##^#SUFFIX#$#_insert(#^#NAME#$##^#SUFFIX#$#* field, size_t idx)\n"
        "{\n"
        "    auto& val = from#^#CONV_SUFFIX#$#(field)->value();\n"
        "    auto nextSize = std::max(idx + 1U, val.size() + 1U);\n"
        "    if (val.capacity() <= nextSize) {\n"
        "        return nullptr;\n"
        "    }\n"
        "    val.resize(nextSize);\n"
        "    return to#^#ELEM_CONV_SUFFIX#$#(&val[idx]);\n"
        "}\n"
        "\n"
        "#^#ELEMENT#$#* #^#NAME#$##^#SUFFIX#$#_append(#^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    auto& val = from#^#CONV_SUFFIX#$#(field)->value();\n"
        "    return #^#NAME#$##^#SUFFIX#$#_insert(field, val.size());\n"
        "}\n"
        "\n"
        "bool #^#NAME#$##^#SUFFIX#$#_remove(#^#NAME#$##^#SUFFIX#$#* field, size_t idx)\n"
        "{\n"
        "    auto& val = from#^#CONV_SUFFIX#$#(field)->value();\n"
        "    if (val.size() <= idx) {\n"
        "        return false;\n"
        "    }\n"
        "    auto iter = val.begin() + idx;\n"
        "    val.erase(iter);\n"
        "    return true;\n"
        "}\n"
    ;

    auto* element = m_cExternalElementField;
    if (element == nullptr) {
        element = m_cMemberElementField;
        assert(element != nullptr);
    }

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"ELEMENT", element->cName()},
        {"CONV_SUFFIX", cConversionSuffix()},
        {"ELEM_CONV_SUFFIX", element->cConversionSuffix()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    if (m_cMemberElementField != nullptr) {
        repl["MEMBER"] = m_cMemberElementField->cSourceCode();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CListField::cCommsHeaderCodeImpl() const
{
    if (m_cMemberElementField != nullptr) {
        return m_cMemberElementField->cCommsHeaderCode();
    }

    return strings::genEmptyString();
}

} // namespace commsdsl2c
