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

#include "CommsBundleField.h"

#include "CommsGenerator.h"
#include "CommsOptionalField.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{
    

CommsBundleField::CommsBundleField(CommsGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CommsBase(static_cast<GenBase&>(*this))
{
}

bool CommsBundleField::genPrepareImpl()
{
    return 
        GenBase::genPrepareImpl() && 
        commsPrepare() &&
        commsPrepareInternal();
}

bool CommsBundleField::genWriteImpl() const
{
    return commsWrite();
}

CommsBundleField::CommsIncludesList CommsBundleField::commsCommonIncludesImpl() const
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

std::string CommsBundleField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsBundleField::commsCommonMembersCodeImpl() const
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

CommsBundleField::CommsIncludesList CommsBundleField::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/field/Bundle.h",
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

std::string CommsBundleField::commsDefMembersCodeImpl() const
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

std::string CommsBundleField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::Bundle<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "    typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = genGenerator();
    auto parseObj = genBundleFieldParseObj();
    util::GenReplacementMap repl = {
        {"PROT_NAMESPACE", gen.genSchemaOf(*this).genMainNamespace()},
        {"CLASS_NAME", comms::genClassName(parseObj.parseName())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    if (comms::genIsGlobalField(*this)) {
        repl["MEMBERS_OPT"] = "<TOpt>";
    }

    return util::genProcessTemplate(Templ, repl);       
}

std::string CommsBundleField::commsDefPublicCodeImpl() const
{
    static const std::string Templ = 
        "#^#ACCESS#$#\n"
        "#^#ALIASES#$#\n";

    util::GenReplacementMap repl = {
        {"ACCESS", commsDefAccessCodeInternal()},
        {"ALIASES", commsDefAliasesCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsBundleField::commsDefPrivateCodeImpl() const
{
    assert(m_bundledReadPrepareCodes.size() == m_commsMembers.size());
    assert(m_bundledRefreshCodes.size() == m_commsMembers.size());
    util::GenStringsList reads;
    util::GenStringsList refreshes;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        auto& readCode = m_bundledReadPrepareCodes[idx];
        auto& refreshCode = m_bundledRefreshCodes[idx];

        if (readCode.empty() && refreshCode.empty()) {
            continue;
        }

        auto accName = comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName());

        if (!readCode.empty()) {
            static const std::string Templ = 
                "void readPrepare_#^#ACC_NAME#$#()\n"
                "{\n"
                "    #^#CODE#$#\n"
                "}\n";

            util::GenReplacementMap repl = {
                {"ACC_NAME", accName},
                {"CODE", readCode}
            };

            reads.push_back(util::genProcessTemplate(Templ, repl));
        }

        if (!refreshCode.empty()) {
            static const std::string Templ = 
                "bool refresh_#^#ACC_NAME#$#()\n"
                "{\n"
                "    #^#CODE#$#\n"
                "}\n";

            util::GenReplacementMap repl = {
                {"ACC_NAME", accName},
                {"CODE", refreshCode}
            };

            refreshes.push_back(util::genProcessTemplate(Templ, repl));
        }
    }

    util::GenStringsList result;
    if (!reads.empty()) {
        result.push_back(util::genStrListToString(reads, "\n", ""));
    }

    if (!refreshes.empty()) {
        result.push_back(util::genStrListToString(refreshes, "\n", ""));
    }
    
    return util::genStrListToString(result, "\n", "");
}

std::string CommsBundleField::commsDefReadFuncBodyImpl() const
{
    util::GenStringsList reads;
    assert(m_bundledReadPrepareCodes.size() == m_commsMembers.size());
    int prevIdx = -1;

    static const std::string EsCheckStr = 
        "if (es != comms::ErrorStatus::Success) {\n"
        "    break;\n"
        "}\n";

    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        if (m_bundledReadPrepareCodes[idx].empty()) {
            continue;
        }

        auto accName = comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName());
        auto prepStr = "readPrepare_" + accName + "();\n";
        if (idx == 0U) {
            reads.push_back(std::move(prepStr));
            continue;
        }

        if (prevIdx < 0) {
            auto str = 
                "es = Base::template readUntilAndUpdateLen<FieldIdx_" + accName + ">(iter, len);\n" + 
                EsCheckStr + '\n' +
                prepStr;
            reads.push_back(std::move(str));
            prevIdx = idx;
            continue;
        }

        auto prevAcc = comms::genAccessName(m_commsMembers[prevIdx]->commsGenField().genParseObj().parseName());
        auto str = 
            "es = Base::template readFromUntilAndUpdateLen<FieldIdx_" + prevAcc + ", FieldIdx_" + accName + ">(iter, len);\n" + 
            EsCheckStr + '\n' +
            prepStr;
        reads.push_back(std::move(str));
        prevIdx = idx;        
    }

    if (reads.empty()) {
        // Members dont have bundled reads
        return strings::genEmptyString();    
    }

    if (prevIdx < 0) {
        // Only the first element has readPrepare()
        reads.push_back("es = Base::read(iter, len);\n");
    }
    else {
        auto prevAcc = comms::genAccessName(m_commsMembers[prevIdx]->commsGenField().genParseObj().parseName());
        reads.push_back("es = Base::template readFrom<FieldIdx_" + prevAcc + ">(iter, len);\n");
    }

    static const std::string Templ = 
        "auto es = comms::ErrorStatus::Success;\n"
        "do {\n"
        "    #^#READS#$#\n"
        "} while(false);\n"
        "return es;";

    util::GenReplacementMap repl = {
        {"READS", util::genStrListToString(reads, "\n", "")},
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsBundleField::commsDefRefreshFuncBodyImpl() const
{
    static const std::string Templ =
        "bool updated = Base::refresh();\n"
        "#^#FIELDS#$#\n"
        "return updated;\n";    

    assert(m_commsMembers.size() == m_bundledRefreshCodes.size());
    util::GenStringsList fields;
    for (auto idx = 0U; idx < m_commsMembers.size(); ++idx) {
        auto& code = m_bundledRefreshCodes[idx];
        if (code.empty()) {
            continue;
        }

        auto accName = comms::genAccessName(m_commsMembers[idx]->commsGenField().genParseObj().parseName());
        fields.push_back("updated = refresh_" + accName + "() || updated;");
    }

    if (fields.empty()) {
        return strings::genEmptyString();
    }
    
    util::GenReplacementMap repl = {
        {"FIELDS", util::genStrListToString(fields, "\n", "")}
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsBundleField::commsDefValidFuncBodyImpl() const
{
    auto validCond = genBundleFieldParseObj().parseValidCond();
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

bool CommsBundleField::commsPrepareInternal()
{
    m_commsMembers = commsTransformFieldsList(genMembers());
    m_bundledReadPrepareCodes.reserve(m_commsMembers.size());
    m_bundledRefreshCodes.reserve(m_commsMembers.size());
    for (auto* m : m_commsMembers) {
        m_bundledReadPrepareCodes.push_back(m->commsDefBundledReadPrepareFuncBody(m_commsMembers));
        m_bundledRefreshCodes.push_back(m->commsDefBundledRefreshFuncBody(m_commsMembers));
    }

    if ((genBundleFieldParseObj().parseSemanticType() == ParseField::ParseSemanticType::Length) && 
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

bool CommsBundleField::commsIsVersionDependentImpl() const
{
    return 
        std::any_of(
            m_commsMembers.begin(), m_commsMembers.end(),
            [](auto* m)
            {
                return m->commsIsVersionDependent();
            });
}

std::string CommsBundleField::commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const
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

std::size_t CommsBundleField::commsMinLengthImpl() const
{
    return
        std::accumulate(
            m_commsMembers.begin(), m_commsMembers.end(), std::size_t(0),
            [](std::size_t soFar, auto* m)
            {
                return comms::genAddLength(soFar, m->commsMinLength());
            });
}

std::size_t CommsBundleField::commsMaxLengthImpl() const
{
    return
        std::accumulate(
            m_commsMembers.begin(), m_commsMembers.end(), std::size_t(0),
            [](std::size_t soFar, auto* m)
            {
                return comms::genAddLength(soFar, m->commsMaxLength());
            });    
}


std::string CommsBundleField::commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const
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

std::string CommsBundleField::commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const
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

void CommsBundleField::commsCompOptChecksImpl(const std::string& accStr, GenStringsList& checks, const std::string& prefix) const
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

std::string CommsBundleField::commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const
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

std::string CommsBundleField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
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

    auto accName = comms::genAccessName(memInfo.first->commsGenField().genParseObj().parseName());
    return memInfo.first->commsCompPrepValueStr(memInfo.second, value);
}

bool CommsBundleField::commsHasCustomLengthDeepImpl() const
{
    return 
        std::any_of(
            m_commsMembers.begin(), m_commsMembers.end(),
            [](auto* m)
            {
                return m->commsHasCustomLength(true);
            });
}

bool CommsBundleField::commsVerifyInnerRefImpl(const std::string& refStr) const
{
    auto info = commsParseMemRefInternal(refStr);
    if (info.first == nullptr) {
        return false;
    }

    return info.first->commsVerifyInnerRef(info.second);
}

std::string CommsBundleField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::GenStringsList opts;
    commsAddFieldDefOptions(opts);
    commsAddCustomReadRefreshOptInternal(opts);
    commsAddRemLengthMemberOptInternal(opts);
    util::genAddToStrList("comms::option::def::HasVersionDependentMembers<" + util::genBoolToString(commsIsVersionDependentImpl()) + ">", opts);        
    return util::genStrListToString(opts, ",\n", "");
}

std::string CommsBundleField::commsDefAccessCodeInternal() const
{
    static const std::string Templ =
        "/// @brief Allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_FIELD_MEMBERS_NAMES macro\n"
        "///     related to @b comms::field::Bundle class from COMMS library\n"
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

std::string CommsBundleField::commsDefAliasesCodeInternal() const
{
    auto obj = genBundleFieldParseObj();
    auto aliases = obj.parseAliases();
    if (aliases.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList result;
    for (auto& a : aliases) {
        static const std::string Templ =
            "/// @brief Alias to a member field.\n"
            "/// @details\n"
            "#^#ALIAS_DESC#$#\n"
            "///     Generates field access alias function(s):\n"
            "///     @b field_#^#ALIAS_NAME#$#() -> <b>#^#ALIASED_FIELD_DOC#$#</b>\n"
            "COMMS_FIELD_ALIAS(#^#ALIAS_NAME#$#, #^#ALIASED_FIELD#$#);\n";
                    
        auto& fieldName = a.parseFieldName();
        assert(!fieldName.empty());
        auto fieldSubNames = util::genStrSplitByAnyChar(fieldName, ".");
        for (auto& n : fieldSubNames) {
            n = comms::genAccessName(n);
        }

        auto desc = util::genStrMakeMultiline(a.parseDescription());
        if (!desc.empty()) {
            desc = strings::genDoxygenPrefixStr() + strings::genIndentStr() + desc + " @n";
            desc = util::genStrReplace(desc, "\n", "\n" + strings::genDoxygenPrefixStr() + strings::genIndentStr());
        }       

        util::GenReplacementMap repl = {
            {"ALIAS_DESC", std::move(desc)},
            {"ALIAS_NAME", comms::genAccessName(a.parseName())},
            {"ALIASED_FIELD_DOC", util::genStrListToString(fieldSubNames, "().field_", "()")},
            {"ALIASED_FIELD", util::genStrListToString(fieldSubNames, ", ", "")}
        };

        result.push_back(util::genProcessTemplate(Templ, repl));                      
    }

    return util::genStrListToString(result, "\n", "");
}

void CommsBundleField::commsAddCustomReadRefreshOptInternal(GenStringsList& opts) const
{
    bool hasGeneratedRead = 
        std::any_of(
            m_bundledReadPrepareCodes.begin(), m_bundledReadPrepareCodes.end(),
            [](const std::string& code)
            {
                return !code.empty();
            });

    bool hasGeneratedRefresh = 
        std::any_of(
            m_bundledRefreshCodes.begin(), m_bundledRefreshCodes.end(),
            [](const std::string& code)
            {
                return !code.empty();
            });

    if (hasGeneratedRead) {
        util::genAddToStrList("comms::option::def::HasCustomRead", opts);
    }

    if (hasGeneratedRefresh) {
        util::genAddToStrList("comms::option::def::HasCustomRefresh", opts);
    }
}

void CommsBundleField::commsAddRemLengthMemberOptInternal(GenStringsList& opts) const
{
    auto lengthFieldIter = 
         std::find_if(
            m_commsMembers.begin(), m_commsMembers.end(),
            [](auto* m) {
                assert(m != nullptr);
                return m->commsGenField().genParseObj().parseSemanticType() == ParseField::ParseSemanticType::Length;
            });   

    if (lengthFieldIter != m_commsMembers.end()) {
        auto idx = static_cast<unsigned>(std::distance(m_commsMembers.begin(), lengthFieldIter));
        auto optStr = "comms::option::def::RemLengthMemberField<" + util::genNumToString(idx) + '>';
        util::genAddToStrList(std::move(optStr), opts);
    }
}

std::pair<const CommsField*, std::string> CommsBundleField::commsParseMemRefInternal(const std::string accStr) const
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
