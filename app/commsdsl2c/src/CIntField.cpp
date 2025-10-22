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

#include "CIntField.h"

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
        "#^#VALUE#$#\n"
        "#^#SPECIALS#$#\n"
        ;

    return Templ;
}

} // namespace

CIntField::CIntField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CIntField::genWriteImpl() const
{
    return cWrite();
}

void CIntField::cAddHeaderIncludesImpl(CIncludesList& includes) const
{
    includes.push_back("<stdint.h>");
}

std::string CIntField::cHeaderCodeImpl() const
{
    util::GenReplacementMap repl = {
        {"VALUE", cHeaderValueCodeInternal()},
        {"SPECIALS", cHeaderSpecialsCodeInternal()},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CIntField::cSourceCodeImpl() const
{
    util::GenReplacementMap repl = {
        {"VALUE", cSourceCommonValueAccessFuncs()},
        {"SPECIALS", cSourceSpecialsCodeInternal()},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CIntField::cFrameValueDefImpl(const std::string& name) const
{
    return cCommonFrameValueDef(cTypeInternal(), name);
}

std::string CIntField::cTypeInternal() const
{
    auto parseObj = genIntFieldParseObj();
    auto cppType = comms::genCppIntTypeFor(parseObj.parseType(), parseObj.parseMaxLength());
    return util::genStrReplace(cppType, "std::", std::string());
}

std::string CIntField::cHeaderValueCodeInternal() const
{
    static const std::string Templ =
        "/// @breif Inner value storage type of @ref #^#NAME#$#.\n"
        "typedef #^#TYPE#$# #^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$#;\n"
        "\n"
        "#^#FUNCS#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"NAME", cName()},
        {"TYPE", cTypeInternal()},
        {"VALUE_TYPE", strings::genValueTypeStr()},
        {"FUNCS", cHeaderCommonValueAccessFuncs()},
    };

    if (cIsVersionOptional()) {
        repl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CIntField::cHeaderSpecialsCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    for (auto& s : specials) {
        if (!genGenerator().genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b> of @ref #^#NAME#$#.\n"
            "#^#SPECIAL_DOC#$#\n"
            "#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# #^#NAME#$##^#SUFFIX#$#_value#^#SPEC_ACC#$#();\n"
            "\n"
            "/// @brief Check the value is equal to special @ref #^#NAME#$##^#SUFFIX#$#_value#^#SPEC_ACC#$#().\n"
            "bool #^#NAME#$##^#SUFFIX#$#_is#^#SPEC_ACC#$#(const #^#NAME#$##^#SUFFIX#$#* field);\n"
            "\n"
            "/// @brief Assign special value @ref #^#NAME#$##^#SUFFIX#$#_value#^#SPEC_ACC#$#() to the field.\n"
            "void #^#NAME#$##^#SUFFIX#$#_set#^#SPEC_ACC#$#(#^#NAME#$##^#SUFFIX#$#* field);\n"
        );

        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = util::genStrMakeMultiline(desc);
            desc = util::genStrReplace(desc, "\n", "\n///     ");
        }

        util::GenReplacementMap repl = {
            {"SPEC_NAME", util::genDisplayName(s.second.m_displayName, s.first)},
            {"SPEC_ACC", comms::genClassName(s.first)},
            {"SPECIAL_DOC", std::move(desc)},
            {"NAME", cName()},
            {"VALUE_TYPE", strings::genValueTypeStr()},
        };

        specialsList.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(specialsList, "\n", "\n");
}

std::string CIntField::cSourceSpecialsCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    for (auto& s : specials) {
        if (!genGenerator().genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ(
            "#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$# #^#NAME#$##^#SUFFIX#$#_value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return static_cast<#^#NAME#$##^#SUFFIX#$#_#^#VALUE_TYPE#$#>(#^#NAME#$##^#SUFFIX#$##^#COMMS_SUFFIX#$#::value#^#SPEC_ACC#$#());\n"
            "}\n"
            "\n"
            "bool #^#NAME#$##^#SUFFIX#$#_is#^#SPEC_ACC#$#(const #^#NAME#$##^#SUFFIX#$#* field)\n"
            "{\n"
            "    return from#^#CONV_SUFFIX#$#(field)->is#^#SPEC_ACC#$#();\n"
            "}\n"
            "void #^#NAME#$##^#SUFFIX#$#_set#^#SPEC_ACC#$#(#^#NAME#$##^#SUFFIX#$#* field)\n"
            "{\n"
            "    from#^#CONV_SUFFIX#$#(field)->set#^#SPEC_ACC#$#();\n"
            "}\n"
        );

        util::GenReplacementMap repl = {
            {"SPEC_ACC", comms::genClassName(s.first)},
            {"NAME", cName()},
            {"VALUE_TYPE", strings::genValueTypeStr()},
            {"COMMS_SUFFIX", strings::genCommsNameSuffixStr()},
            {"CONV_SUFFIX", cConversionSuffix()},
        };

        specialsList.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(specialsList, "\n", "\n");
}

} // namespace commsdsl2c
