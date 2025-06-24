//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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
#include "CommsOptionalField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{
    

CommsBitfieldField::CommsBitfieldField(
    CommsGenerator& generator, 
    commsdsl::parse::ParseField dslObj, 
    commsdsl::gen::GenElem* parent) :
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
        names.push_back(comms::className(fPtr->dslObj().parseName()));
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
        {"PROT_NAMESPACE", gen.schemaOf(*this).mainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(dslObj.parseEndian())},
        {"CLASS_NAME", comms::className(dslObj.parseName())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = std::string(",");
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

std::string CommsBitfieldField::commsDefValidFuncBodyImpl() const
{
    auto validCond = bitfieldDslObj().parseValidCond();
    if (!validCond.parseValid()) {
        return strings::emptyString();
    }

    auto& gen = CommsGenerator::cast(generator());
    auto str = CommsOptionalField::commsDslCondToString(gen, m_members, validCond, true);    

    if (str.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "if (!Base::valid()) {\n"
        "    return false;\n"
        "}\n\n"
        "return\n"
        "    #^#CODE#$#;\n"
        ;

    util::ReplacementMap repl = {
        {"CODE", std::move(str)},
    };

    return util::processTemplate(Templ, repl);    
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

std::string CommsBitfieldField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
{
    assert(fieldOptsFunc != nullptr);
    util::StringsList elems;
    for (auto* m : m_members) {
        auto str = (m->*fieldOptsFunc)();
        if (!str.empty()) {
            elems.push_back(std::move(str));
        }
    }
    return util::strListToString(elems, "\n", "");
}

std::string CommsBitfieldField::commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsValueAccessStrImpl(accStr, prefix);
    }

    auto memInfo = parseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::unexpectedValueStr();
    }

    return memInfo.first->commsValueAccessStr(memInfo.second, prefix + ".field_" + comms::accessName(memInfo.first->field().dslObj().parseName()) + "()");
}

std::string CommsBitfieldField::commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsSizeAccessStrImpl(accStr, prefix);
    }

    auto memInfo = parseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::unexpectedValueStr();
    }

    return memInfo.first->commsSizeAccessStr(memInfo.second, prefix + ".field_" + comms::accessName(memInfo.first->field().dslObj().parseName()) + "()");
}

void CommsBitfieldField::commsCompOptChecksImpl(const std::string& accStr, StringsList& checks, const std::string& prefix) const
{
    if (accStr.empty()) {
        CommsBase::commsCompOptChecksImpl(accStr, checks, prefix);
        return;
    }

    auto memInfo = parseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return;
    }

    return memInfo.first->commsCompOptChecks(memInfo.second, checks, prefix + ".field_" + comms::accessName(memInfo.first->field().dslObj().parseName()) + "()");
}

std::string CommsBitfieldField::commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsCompValueCastTypeImpl(accStr, prefix);
    }

    auto memInfo = parseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::unexpectedValueStr();
    }

    auto accName = comms::accessName(memInfo.first->field().dslObj().parseName());
    return memInfo.first->commsCompValueCastType(memInfo.second, prefix + "Field_" + accName + "::");
}

std::string CommsBitfieldField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
{
    if (accStr.empty()) {
        return CommsBase::commsCompPrepValueStrImpl(accStr, value);
    }

    auto memInfo = parseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return value;
    }

    return memInfo.first->commsCompPrepValueStr(memInfo.second, value);
}

bool CommsBitfieldField::commsHasCustomLengthDeepImpl() const
{
    return 
        std::any_of(
            m_members.begin(), m_members.end(),
            [](auto* m)
            {
                return m->commsHasCustomLength(true);
            });
}

bool CommsBitfieldField::commsVerifyInnerRefImpl(const std::string& refStr) const
{
    auto info = parseMemRefInternal(refStr);
    if (info.first == nullptr) {
        return false;
    }

    return info.first->commsVerifyInnerRef(info.second);
}

bool CommsBitfieldField::commsPrepareInternal()
{
    m_members = commsTransformFieldsList(members());

    if ((bitfieldDslObj().parseSemanticType() == commsdsl::parse::ParseField::SemanticType::Length) && 
        (!commsHasCustomValue())) {
        generator().logger().warning(
            "Field \"" + comms::scopeFor(*this, generator()) + "\" is used as \"length\" field (semanticType=\"length\"), but custom value "
            "retrieval functionality is not provided. Please create relevant code injection functionality with \"" + 
            strings::valueFileSuffixStr() + "\" file name suffix. Inside that file the following functions are "
            "expected to be defined: getValue(), setValue(), and maxValue()."
        );
    }    
    return true;
}

std::string CommsBitfieldField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::StringsList opts;
    commsAddFieldDefOptions(opts);
    util::addToStrList("comms::option::def::HasVersionDependentMembers<" + util::boolToString(commsIsVersionDependentImpl()) + ">", opts);        
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
        namesList.push_back(comms::accessName(mPtr->dslObj().parseName()));
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

std::pair<const CommsField*, std::string> CommsBitfieldField::parseMemRefInternal(const std::string& accStr) const
{
    auto sepPos = accStr.find(".");
    auto memberName = accStr.substr(0, sepPos);

    auto iter = 
        std::find_if(
            m_members.begin(), m_members.end(),
            [&memberName](auto* mem)
            {
                return mem->field().dslObj().parseName() == memberName;
            });

    if (iter == m_members.end()) {
        return std::make_pair(nullptr, accStr);
    }

    std::string remAccStr;
    if (sepPos < accStr.size()) {
        remAccStr = accStr.substr(sepPos + 1);
    }

    return std::make_pair(*iter, std::move(remAccStr));
}

} // namespace commsdsl2comms
