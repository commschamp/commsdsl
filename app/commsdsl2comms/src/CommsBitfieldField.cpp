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

CommsBitfieldField::CommsBitfieldField(CommsGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CommsBase(static_cast<GenBase&>(*this))
{
}

bool CommsBitfieldField::genPrepareImpl()
{
    return
        GenBase::genPrepareImpl() &&
        commsPrepare() &&
        commsPrepareInternal();
}

bool CommsBitfieldField::genWriteImpl() const
{
    return commsWrite();
}

CommsBitfieldField::CommsIncludesList CommsBitfieldField::commsCommonIncludesImpl() const
{
    CommsIncludesList result;
    for (auto* m : m_commsMembers) {
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
    util::GenStringsList membersCode;
    for (auto* m : m_commsMembers) {
        auto code = m->commsCommonCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }

    return util::genStrListToString(membersCode, "\n", "");
}

CommsBitfieldField::CommsIncludesList CommsBitfieldField::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/field/Bitfield.h",
        "<tuple>"
    };

    for (auto* m : m_commsMembers) {
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

    util::GenStringsList membersCode;
    for (auto* m : m_commsMembers) {
        auto code = m->commsDefCode();
        if (!code.empty()) {
            membersCode.push_back(std::move(code));
        }
    }

    util::GenStringsList names;
    for (auto& fPtr : genMembers()) {
        assert(fPtr);
        names.push_back(comms::genClassName(fPtr->genParseObj().parseName()));
    }

    util::GenReplacementMap repl = {
        {"MEMBERS_DEFS", util::genStrListToString(membersCode, "\n", "")},
        {"MEMBERS", util::genStrListToString(names, ",\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsBitfieldField::commsDefBaseClassImpl() const
{
    static const std::string Templ =
        "comms::field::Bitfield<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";

    auto& gen = genGenerator();
    auto parseObj = genBitfieldFieldParseObj();
    util::GenReplacementMap repl = {
        {"PROT_NAMESPACE", gen.genSchemaOf(*this).genMainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(parseObj.parseEndian())},
        {"CLASS_NAME", comms::genClassName(parseObj.parseName())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = std::string(",");
    }

    if (comms::genIsGlobalField(*this)) {
        repl["MEMBERS_OPT"] = "<TOpt>";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsBitfieldField::commsDefPublicCodeImpl() const
{
    return commsAccessCodeInternal();
}

std::string CommsBitfieldField::commsDefValidFuncBodyImpl() const
{
    auto validCond = genBitfieldFieldParseObj().parseValidCond();
    if (!validCond.parseValid()) {
        return strings::genEmptyString();
    }

    auto& gen = CommsGenerator::commsCast(genGenerator());
    auto str = CommsOptionalField::commsDslCondToString(gen, m_commsMembers, validCond, true);

    if (str.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "if (!Base::valid()) {\n"
        "    return false;\n"
        "}\n\n"
        "return\n"
        "    #^#CODE#$#;\n"
        ;

    util::GenReplacementMap repl = {
        {"CODE", std::move(str)},
    };

    return util::genProcessTemplate(Templ, repl);
}

bool CommsBitfieldField::commsIsVersionDependentImpl() const
{
    return
        std::any_of(
            m_commsMembers.begin(), m_commsMembers.end(),
            [](auto* m)
            {
                return m->commsIsVersionDependent();
            });
}

std::string CommsBitfieldField::commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const
{
    assert(fieldOptsFunc != nullptr);
    util::GenStringsList elems;
    for (auto* m : m_commsMembers) {
        auto str = (m->*fieldOptsFunc)();
        if (!str.empty()) {
            elems.push_back(std::move(str));
        }
    }
    return util::genStrListToString(elems, "\n", "");
}

std::string CommsBitfieldField::commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsValueAccessStrImpl(accStr, prefix);
    }

    auto memInfo = commsParseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genUnexpectedValueStr();
    }

    return memInfo.first->commsValueAccessStr(memInfo.second, prefix + ".field_" + comms::genAccessName(memInfo.first->commsGenField().genParseObj().parseName()) + "()");
}

std::string CommsBitfieldField::commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsSizeAccessStrImpl(accStr, prefix);
    }

    auto memInfo = commsParseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genUnexpectedValueStr();
    }

    return memInfo.first->commsSizeAccessStr(memInfo.second, prefix + ".field_" + comms::genAccessName(memInfo.first->commsGenField().genParseObj().parseName()) + "()");
}

void CommsBitfieldField::commsCompOptChecksImpl(const std::string& accStr, GenStringsList& checks, const std::string& prefix) const
{
    if (accStr.empty()) {
        CommsBase::commsCompOptChecksImpl(accStr, checks, prefix);
        return;
    }

    auto memInfo = commsParseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return;
    }

    return memInfo.first->commsCompOptChecks(memInfo.second, checks, prefix + ".field_" + comms::genAccessName(memInfo.first->commsGenField().genParseObj().parseName()) + "()");
}

std::string CommsBitfieldField::commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsCompValueCastTypeImpl(accStr, prefix);
    }

    auto memInfo = commsParseMemRefInternal(accStr);

    if (memInfo.first == nullptr) {
        [[maybe_unused]] static const bool Should_not_happen = false;
        assert(Should_not_happen);
        return strings::genUnexpectedValueStr();
    }

    auto accName = comms::genAccessName(memInfo.first->commsGenField().genParseObj().parseName());
    return memInfo.first->commsCompValueCastType(memInfo.second, prefix + "Field_" + accName + "::");
}

std::string CommsBitfieldField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
{
    if (accStr.empty()) {
        return CommsBase::commsCompPrepValueStrImpl(accStr, value);
    }

    auto memInfo = commsParseMemRefInternal(accStr);

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
            m_commsMembers.begin(), m_commsMembers.end(),
            [](auto* m)
            {
                return m->commsHasCustomLength(true);
            });
}

bool CommsBitfieldField::commsVerifyInnerRefImpl(const std::string& refStr) const
{
    auto info = commsParseMemRefInternal(refStr);
    if (info.first == nullptr) {
        return false;
    }

    return info.first->commsVerifyInnerRef(info.second);
}

bool CommsBitfieldField::commsPrepareInternal()
{
    m_commsMembers = commsTransformFieldsList(genMembers());

    if ((genBitfieldFieldParseObj().parseSemanticType() == ParseField::ParseSemanticType::Length) &&
        (!commsHasCustomValue())) {
        genGenerator().genLogger().genWarning(
            "Field \"" + comms::genScopeFor(*this, genGenerator()) + "\" is used as \"length\" field (semanticType=\"length\"), but custom value "
            "retrieval functionality is not provided. Please create relevant code injection functionality with \"" +
            strings::genValueFileSuffixStr() + "\" file name suffix. Inside that file the following functions are "
            "expected to be defined: getValue(), setValue(), and maxValue()."
        );
    }
    return true;
}

std::string CommsBitfieldField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::GenStringsList opts;
    commsAddFieldDefOptions(opts);
    util::genAddToStrList("comms::option::def::HasVersionDependentMembers<" + util::genBoolToString(commsIsVersionDependentImpl()) + ">", opts);
    return util::genStrListToString(opts, ",\n", "");
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

    util::GenStringsList accessDocList;
    util::GenStringsList namesList;
    accessDocList.reserve(m_commsMembers.size());
    namesList.reserve(m_commsMembers.size());

    auto& gen = genGenerator();
    for (auto& mPtr : genMembers()) {
        namesList.push_back(comms::genAccessName(mPtr->genParseObj().parseName()));
        std::string accessStr =
            "///     @li @b FieldIdx_" + namesList.back() +
            " index, @b Field_" + namesList.back() +
            " type and @b field_" + namesList.back() +
            "() access function -\n"
            "///         for " + comms::genScopeFor(*mPtr, gen) + " member field.";
        accessDocList.push_back(std::move(accessStr));
    }

    util::GenReplacementMap repl = {
        {"ACCESS_DOC", util::genStrListToString(accessDocList, "\n", "")},
        {"NAMES", util::genStrListToString(namesList, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::pair<const CommsField*, std::string> CommsBitfieldField::commsParseMemRefInternal(const std::string& accStr) const
{
    auto sepPos = accStr.find(".");
    auto memberName = accStr.substr(0, sepPos);

    auto iter =
        std::find_if(
            m_commsMembers.begin(), m_commsMembers.end(),
            [&memberName](auto* mem)
            {
                return mem->commsGenField().genParseObj().parseName() == memberName;
            });

    if (iter == m_commsMembers.end()) {
        return std::make_pair(nullptr, accStr);
    }

    std::string remAccStr;
    if (sepPos < accStr.size()) {
        remAccStr = accStr.substr(sepPos + 1);
    }

    return std::make_pair(*iter, std::move(remAccStr));
}

} // namespace commsdsl2comms
