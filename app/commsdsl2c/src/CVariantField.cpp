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

#include "CVariantField.h"

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

const std::string FieldIdxStr("FieldIdx");
const std::string& cCodeTemplInternal()
{
    static const std::string Templ =
        "#^#MEMBERS#$#\n"
        "#^#ACCESS#$#\n"
        "#^#EXTRA#$#\n"
        ;

    return Templ;
}

} // namespace

CVariantField::CVariantField(CGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CBase(static_cast<GenBase&>(*this))
{
}

bool CVariantField::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_cMembers = cTransformFieldsList(genMembers());
    return true;
}

bool CVariantField::genWriteImpl() const
{
    return cWrite();
}


void CVariantField::cAddHeaderIncludesImpl(CIncludesList& includes) const
{
    for (auto* m : m_cMembers) {
        m->cAddHeaderIncludes(includes);
    }
}

void CVariantField::cAddSourceIncludesImpl(CIncludesList& includes) const
{
    includes.push_back("<algorithm>");

    for (auto* m : m_cMembers) {
        m->cAddSourceIncludes(includes);
    }
}

void CVariantField::cAddCommsHeaderIncludesImpl(CIncludesList& includes) const
{
    for (auto* m : m_cMembers) {
        m->cAddCommsHeaderIncludes(includes);
    }
}

std::string CVariantField::cHeaderCodeImpl() const
{
    GenStringsList members;
    GenStringsList access;
    GenStringsList indices;
    GenStringsList handles;

    for (auto* m : m_cMembers) {
        members.push_back(m->cHeaderCode());

        static const std::string AccTempl =
            "/// @brief Initialize and acquire access to member @ref #^#MEM#$# of @ref #^#NAME#$##^#SUFFIX#$#.\n"
            "#^#MEM#$#* #^#NAME#$##^#SUFFIX#$#_initField_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field);\n"
            "\n"
            "/// @brief Deinitialize access to member @ref #^#MEM#$# of @ref #^#NAME#$##^#SUFFIX#$#.\n"
            "void #^#NAME#$##^#SUFFIX#$#_deinitField_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field);\n"
            "\n"
            "/// @brief Acquire access to already initialised member @ref #^#MEM#$# of @ref #^#NAME#$##^#SUFFIX#$#.\n"
            "#^#MEM#$#* #^#NAME#$##^#SUFFIX#$#_accessField_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field);\n"
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

        auto memObj = m->cGenField().genParseObj();
        auto memDispName = util::genDisplayName(memObj.parseDisplayName(), memObj.parseName());
        indices.push_back(cName() + "_" + FieldIdxStr + "_" + accRepl["MEM_NAME"] + ", ///< Index of the <b>" + memDispName + "</b>.");
        handles.push_back("void (*handle_" + accRepl["MEM_NAME"] + ")(" + m->cName() + "* field);");
    }

    static const std::string ExtraTempl = 
        "/// @brief Access indices for the members of the @ref #^#NAME#$##^#SUFFIX#$# field.\n"
        "typedef enum\n"
        "{\n"
        "    #^#IDX#$#\n"
        "    #^#NAME#$#_#^#FIELD_IDX#$#_numOfValues ///< Limit to the available members.\n"
        "} #^#NAME#$#_#^#FIELD_IDX#$#;\n"
        "\n"
        "/// @brief Get an index of the currently selected (initialized) member.\n"
        "#^#NAME#$#_#^#FIELD_IDX#$# #^#NAME#$##^#SUFFIX#$#_currentField(const #^#NAME#$##^#SUFFIX#$#* field);\n"    
        "\n"
        "/// @brief Select a member field of the @ref #^#NAME#$##^#SUFFIX#$# to initialize by its index.\n"   
        "void #^#NAME#$##^#SUFFIX#$#_selectField(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$#_#^#FIELD_IDX#$# idx);\n"   
        "\n"
        "/// @brief Deinitialize currently selected member of @ref #^#NAME#$##^#SUFFIX#$#.\n"
        "void #^#NAME#$##^#SUFFIX#$#_reset(#^#NAME#$##^#SUFFIX#$#*);\n"
        "\n"
        "/// @brief Handler for the members of the @ref #^#NAME#$##^#SUFFIX#$#.\n"
        "typedef struct\n"
        "{\n"
        "    #^#HANDLES#$#\n"
        "} #^#NAME#$#_MemHandler;\n"
        "\n"
        "/// @brief Dispatch the currently selected member into its appropriate handling function.\n"
        "void #^#NAME#$##^#SUFFIX#$#_currFieldExec(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$#_MemHandler* handler);\n"
        ;

    util::GenReplacementMap extraRepl = {
        {"IDX", util::genStrListToString(indices, "\n", "")},
        {"HANDLES", util::genStrListToString(handles, "\n", "")},
        {"NAME", cName()},
        {"FIELD_IDX", FieldIdxStr},
    };

    if (cIsVersionOptional()) {
        extraRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }    

    util::GenReplacementMap repl = {
        {"MEMBERS", util::genStrListToString(members, "\n", "\n")},
        {"ACCESS", util::genStrListToString(access, "\n", "\n")},
        {"EXTRA", util::genProcessTemplate(ExtraTempl, extraRepl)},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CVariantField::cSourceCodeImpl() const
{
    GenStringsList members;
    GenStringsList access;
    GenStringsList handles;

    for (auto* m : m_cMembers) {
        members.push_back(m->cSourceCode());

        static const std::string AccTempl =
            "#^#MEM#$#* #^#NAME#$##^#SUFFIX#$#_initField_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field)\n"
            "{\n"
            "    return to#^#MEM_CONV_SUFFIX#$#(&(from#^#CONV_SUFFIX#$#(field)->initField_#^#MEM_NAME#$#()));\n"
            "}\n"
            "\n"
            "void #^#NAME#$##^#SUFFIX#$#_deinitField_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field)\n"
            "{\n"
            "    from#^#CONV_SUFFIX#$#(field)->deinitField_#^#MEM_NAME#$#();\n"
            "}\n"
            "\n"
            "#^#MEM#$#* #^#NAME#$##^#SUFFIX#$#_accessField_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#* field)\n"
            "{\n"
            "    return to#^#MEM_CONV_SUFFIX#$#(&(from#^#CONV_SUFFIX#$#(field)->accessField_#^#MEM_NAME#$#()));\n"
            "}\n"  
        ;

        util::GenReplacementMap accRepl = {
            {"NAME", cName()},
            {"MEM", m->cName()},
            {"MEM_NAME", comms::genAccessName(m->cGenField().genName())},
            {"CONV_SUFFIX", cConversionSuffix()},
            {"MEM_CONV_SUFFIX", m->cConversionSuffix()},
            {"FIELD_IDX", FieldIdxStr},
        };

        if (cIsVersionOptional()) {
            accRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
        }

        access.push_back(util::genProcessTemplate(AccTempl, accRepl));

        const std::string HandleTempl = 
            "case #^#NAME#$#_#^#FIELD_IDX#$#_#^#MEM_NAME#$#:\n"
            "    return handler->handle_#^#MEM_NAME#$#(#^#NAME#$##^#SUFFIX#$#_accessField_#^#MEM_NAME#$#(field));"
            ;

        handles.push_back(util::genProcessTemplate(HandleTempl, accRepl));
    }

    static const std::string ExtraTempl = 
        "#^#NAME#$#_#^#FIELD_IDX#$# #^#NAME#$##^#SUFFIX#$#_currentField(const #^#NAME#$##^#SUFFIX#$#* field)\n"          
        "{\n"
        "    return static_cast<#^#NAME#$#_#^#FIELD_IDX#$#>(std::min(static_cast<std::size_t>(#^#NAME#$#_#^#FIELD_IDX#$#_numOfValues), from#^#CONV_SUFFIX#$#(field)->currentField()));\n"
        "}\n"   
        "\n" 
        "void #^#NAME#$##^#SUFFIX#$#_selectField(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$#_#^#FIELD_IDX#$# idx)\n"
        "{\n"
        "    from#^#CONV_SUFFIX#$#(field)->selectField(static_cast<std::size_t>(idx));\n"
        "}\n"  
        "\n" 
        "void #^#NAME#$##^#SUFFIX#$#_reset(#^#NAME#$##^#SUFFIX#$#* field)\n"
        "{\n"
        "    from#^#CONV_SUFFIX#$#(field)->reset();\n"
        "}\n"
        "\n"                       
        "void #^#NAME#$##^#SUFFIX#$#_currFieldExec(#^#NAME#$##^#SUFFIX#$#* field, #^#NAME#$#_MemHandler* handler)\n"
        "{\n"
        "   if (handler == nullptr) {\n"
        "       return;\n"
        "   }\n\n"
        "   switch(#^#NAME#$##^#SUFFIX#$#_currentField(field)) {\n"
        "       #^#CASES#$#\n"
        "       default: break;\n"
        "   }\n"
        "}\n"
        ;

    util::GenReplacementMap extraRepl = {
        {"NAME", cName()},
        {"FIELD_IDX", FieldIdxStr},
        {"CONV_SUFFIX", cConversionSuffix()},
        {"CASES", util::genStrListToString(handles, "\n", "")},
    };

    if (cIsVersionOptional()) {
        extraRepl["SUFFIX"] = strings::genVersionOptionalFieldSuffixStr();
    }        

    util::GenReplacementMap repl = {
        {"MEMBERS", util::genStrListToString(members, "\n", "\n")},
        {"ACCESS", util::genStrListToString(access, "\n", "\n")},
        {"EXTRA", util::genProcessTemplate(ExtraTempl, extraRepl)},
    };

    return util::genProcessTemplate(cCodeTemplInternal(), repl);
}

std::string CVariantField::cCommsHeaderCodeImpl() const
{
    GenStringsList members;
    for (auto* m : m_cMembers) {
        members.push_back(m->cCommsHeaderCode());
    }
    return util::genStrListToString(members, "\n", "\n");
}

} // namespace commsdsl2c
