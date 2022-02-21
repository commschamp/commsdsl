//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsBitfieldField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2new
{
    

CommsBitfieldField::CommsBitfieldField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsBitfieldField::prepareImpl()
{
    return 
        Base::prepareImpl() && 
        commsPrepare() &&
        commsPrepareInternal();
}

bool CommsBitfieldField::writeImpl() const
{
    return commsWrite();
}

CommsBitfieldField::IncludesList CommsBitfieldField::commsCommonIncludesImpl() const
{
    IncludesList result;
    for (auto* m : m_members) {
        assert(m != nullptr);
        auto incList = m->commsCommonIncludes();
        result.reserve(result.size() + incList.size());
        std::move(incList.begin(), incList.end(), std::back_inserter(result));
    } 
    return result;
}

std::string CommsBitfieldField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsBitfieldField::commsCommonMembersCodeImpl() const
{
    util::StringsList membersCode;
    for (auto* m : m_members) {
        auto code = m->commsCommonCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }

    return util::strListToString(membersCode, "\n", "");
}

CommsBitfieldField::IncludesList CommsBitfieldField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/Bitfield.h",
        "<tuple>"        
    };
    
    for (auto* m : m_members) {
        assert(m != nullptr);
        auto incList = m->commsDefIncludes();
        result.reserve(result.size() + incList.size());
        std::move(incList.begin(), incList.end(), std::back_inserter(result));
    } 
    return result;
}

std::string CommsBitfieldField::commsDefMembersCodeImpl() const
{
    static const std::string Templ = 
        "#^#MEMBERS_DEFS#$#\n"
        "/// @brief All members bundled in @b std::tuple.\n"
        "using All =\n"
        "    std::tuple<\n"
        "       #^#MEMBERS#$#\n"
        "    >;";

    util::StringsList membersCode;
    for (auto* m : m_members) {
        auto code = m->commsDefCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }
    
    util::StringsList names;
    for (auto& fPtr : members()) {
        assert(fPtr);
        names.push_back(comms::className(fPtr->dslObj().name()));
    }

    util::ReplacementMap repl = {
        {"MEMBERS_DEFS", util::strListToString(membersCode, "\n", "")},
        {"MEMBERS", util::strListToString(names, ",\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsBitfieldField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::Bitfield<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = generator();
    auto dslObj = bitfieldDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.mainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(dslObj.endian())},
        {"CLASS_NAME", comms::className(dslObj.name())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    if (comms::isGlobalField(*this)) {
        repl["MEMBERS_OPT"] = "<TOpt>";
    }

    return util::processTemplate(Templ, repl);       
}

std::string CommsBitfieldField::commsDefPublicCodeImpl() const
{
    return commsAccessCodeInternal();
}

bool CommsBitfieldField::commsIsVersionDependentImpl() const
{
    return 
        std::any_of(
            m_members.begin(), m_members.end(),
            [](auto* m)
            {
                return m->commsIsVersionDependent();
            });
}

bool CommsBitfieldField::commsPrepareInternal()
{
    m_members = commsTransformFieldsList(members());
    return true;
}

std::string CommsBitfieldField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::StringsList opts;
    commsAddFieldDefOptions(opts);
    return util::strListToString(opts, ",\n", "");
}

std::string CommsBitfieldField::commsAccessCodeInternal() const
{
    static const std::string Templ =
        "/// @brief Allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_FIELD_MEMBERS_NAMES macro\n"
        "///     related to @b comms::field::Bitfield class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated values, types and access functions are:\n"
        "#^#ACCESS_DOC#$#\n"
        "COMMS_FIELD_MEMBERS_NAMES(\n"
        "    #^#NAMES#$#\n"
        ");\n";

    util::StringsList accessDocList;
    util::StringsList namesList;
    accessDocList.reserve(m_members.size());
    namesList.reserve(m_members.size());

    auto& gen = generator();
    for (auto& mPtr : members()) {
        namesList.push_back(comms::accessName(mPtr->dslObj().name()));
        std::string accessStr =
            "///     @li @b FieldIdx_" + namesList.back() +
            " index, @b Field_" + namesList.back() +
            " type and @b field_" + namesList.back() +
            "() access function -\n"
            "///         for " + comms::scopeFor(*mPtr, gen) + " member field.";
        accessDocList.push_back(std::move(accessStr));
    }

    util::ReplacementMap repl = {
        {"ACCESS_DOC", util::strListToString(accessDocList, "\n", "")},
        {"NAMES", util::strListToString(namesList, ",\n", "")},
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2new
