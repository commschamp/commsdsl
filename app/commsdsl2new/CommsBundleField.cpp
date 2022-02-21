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

#include "CommsBundleField.h"

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
    

CommsBundleField::CommsBundleField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsBundleField::prepareImpl()
{
    return 
        Base::prepareImpl() && 
        commsPrepare() &&
        commsPrepareInternal();
}

bool CommsBundleField::writeImpl() const
{
    return commsWrite();
}

CommsBundleField::IncludesList CommsBundleField::commsCommonIncludesImpl() const
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

std::string CommsBundleField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsBundleField::commsCommonMembersCodeImpl() const
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

CommsBundleField::IncludesList CommsBundleField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/Bundle.h",
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

std::string CommsBundleField::commsDefMembersCodeImpl() const
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

std::string CommsBundleField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::Bundle<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "    typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = generator();
    auto dslObj = bundleDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.mainNamespace()},
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

std::string CommsBundleField::commsDefPublicCodeImpl() const
{
    static const std::string Templ = 
        "#^#ACCESS#$#\n"
        "#^#ALIASES#$#\n";

    util::ReplacementMap repl = {
        {"ACCESS", commsDefAccessCodeInternal()},
        {"ALIASES", commsDefAliasesCodeInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsBundleField::commsDefPrivateCodeImpl() const
{
    assert(m_bundledReadPrepareCodes.size() == m_members.size());
    assert(m_bundledRefreshCodes.size() == m_members.size());
    util::StringsList reads;
    util::StringsList refreshes;
    for (auto idx = 0U; idx < m_members.size(); ++idx) {
        auto& readCode = m_bundledReadPrepareCodes[idx];
        auto& refreshCode = m_bundledRefreshCodes[idx];

        if (readCode.empty() && refreshCode.empty()) {
            continue;
        }

        auto accName = comms::accessName(m_members[idx]->field().dslObj().name());

        if (!readCode.empty()) {
            static const std::string Templ = 
                "void readPrepare_#^#ACC_NAME#$#()\n"
                "{\n"
                "    #^#CODE#$#\n"
                "}\n";

            util::ReplacementMap repl = {
                {"ACC_NAME", accName},
                {"CODE", readCode}
            };

            reads.push_back(util::processTemplate(Templ, repl));
        }

        if (!refreshCode.empty()) {
            static const std::string Templ = 
                "bool refresh_#^#ACC_NAME#$#()\n"
                "{\n"
                "    #^#CODE#$#\n"
                "}\n";

            util::ReplacementMap repl = {
                {"ACC_NAME", accName},
                {"CODE", refreshCode}
            };

            refreshes.push_back(util::processTemplate(Templ, repl));
        }
    }

    util::StringsList result;
    if (!reads.empty()) {
        result.push_back(util::strListToString(reads, "\n", ""));
    }

    if (!refreshes.empty()) {
        result.push_back(util::strListToString(refreshes, "\n", ""));
    }
    
    return util::strListToString(result, "\n", "");
}

std::string CommsBundleField::commsDefReadFuncBodyImpl() const
{
    util::StringsList reads;
    assert(m_bundledReadPrepareCodes.size() == m_members.size());
    int prevIdx = -1;

    static const std::string EsCheckStr = 
        "if (es != comms::ErrorStatus::Success) {\n"
        "    break;\n"
        "}\n";

    for (auto idx = 0U; idx < m_members.size(); ++idx) {
        if (m_bundledReadPrepareCodes[idx].empty()) {
            continue;
        }

        auto accName = comms::accessName(m_members[idx]->field().dslObj().name());
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

        auto prevAcc = comms::accessName(m_members[prevIdx]->field().dslObj().name());
        auto str = 
            "es = Base::template readFromUntilAndUpdateLen<FieldIdx_" + prevAcc + ", FieldIdx_" + accName + ">(iter, len);\n" + 
            EsCheckStr + '\n' +
            prepStr;
        reads.push_back(std::move(str));
        prevIdx = idx;        
    }

    if (reads.empty()) {
        // Members dont have bundled reads
        return strings::emptyString();    
    }

    if (prevIdx < 0) {
        // Only the first element has readPrepare()
        reads.push_back("es = Base::read(iter, len);\n");
    }
    else {
        auto prevAcc = comms::accessName(m_members[prevIdx]->field().dslObj().name());
        reads.push_back("es = Base::teamplate readFrom<FieldIdx_" + prevAcc + ">(iter, len);\n");
    }

    static const std::string Templ = 
        "auto es = comms::ErrorStatus::Success;\n"
        "do {\n"
        "    #^#READS#$#\n"
        "} while(false);\n"
        "return es;";

    util::ReplacementMap repl = {
        {"READS", util::strListToString(reads, "\n", "")},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsBundleField::commsDefRefreshFuncBodyImpl() const
{
    static const std::string Templ =
        "bool updated = Base::refresh();\n"
        "#^#FIELDS#$#\n"
        "return updated;";    

    assert(m_members.size() == m_bundledRefreshCodes.size());
    util::StringsList fields;
    for (auto idx = 0U; idx < m_members.size(); ++idx) {
        auto& code = m_bundledRefreshCodes[idx];
        if (code.empty()) {
            continue;
        }

        auto accName = comms::accessName(m_members[idx]->field().dslObj().name());
        fields.push_back("updated = refresh_" + accName + "() || updated;");
    }

    if (fields.empty()) {
        return strings::emptyString();
    }
    
    util::ReplacementMap repl = {
        {"FIELDS", util::strListToString(fields, "\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

bool CommsBundleField::commsPrepareInternal()
{
    m_members = commsTransformFieldsList(members());
    m_bundledReadPrepareCodes.reserve(m_members.size());
    m_bundledRefreshCodes.reserve(m_members.size());
    for (auto* m : m_members) {
        m_bundledReadPrepareCodes.push_back(m->commsDefBundledReadPrepareFuncBody(m_members));
        m_bundledRefreshCodes.push_back(m->commsDefBundledRefreshFuncBody(m_members));
    }
    return true;
}

bool CommsBundleField::commsIsVersionDependentImpl() const
{
    return 
        std::any_of(
            m_members.begin(), m_members.end(),
            [](auto* m)
            {
                return m->commsIsVersionDependent();
            });
}

std::string CommsBundleField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::StringsList opts;
    commsAddFieldDefOptions(opts);
    commsAddCustomReadRefreshOptInternal(opts);
    return util::strListToString(opts, ",\n", "");
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

std::string CommsBundleField::commsDefAliasesCodeInternal() const
{
    auto obj = bundleDslObj();
    auto aliases = obj.aliases();
    if (aliases.empty()) {
        return strings::emptyString();
    }

    util::StringsList result;
    for (auto& a : aliases) {
        static const std::string Templ =
            "/// @brief Alias to a member field.\n"
            "/// @details\n"
            "#^#ALIAS_DESC#$#\n"
            "///     Generates field access alias function(s):\n"
            "///     @b field_#^#ALIAS_NAME#$#() -> <b>#^#ALIASED_FIELD_DOC#$#</b>\n"
            "COMMS_FIELD_ALIAS(#^#ALIAS_NAME#$#, #^#ALIASED_FIELD#$#);\n";
                    
        auto& fieldName = a.fieldName();
        assert(!fieldName.empty());
        auto fieldSubNames = util::strSplitByAnyCharCompressed(fieldName, ".");
        for (auto& n : fieldSubNames) {
            n = comms::accessName(n);
        }

        auto desc = util::strMakeMultiline(a.description());
        if (!desc.empty()) {
            desc = strings::doxygenPrefixStr() + strings::indentStr() + desc + " @n";
            desc = util::strReplace(desc, "\n", "\n" + strings::doxygenPrefixStr() + strings::indentStr());
        }       

        util::ReplacementMap repl = {
            {"ALIAS_DESC", std::move(desc)},
            {"ALIAS_NAME", comms::accessName(a.name())},
            {"ALIASED_FIELD_DOC", util::strListToString(fieldSubNames, "().field_", "()")},
            {"ALIASED_FIELD", util::strListToString(fieldSubNames, ", ", "")}
        };

        result.push_back(util::processTemplate(Templ, repl));                      
    }

    return util::strListToString(result, "\n", "");
}

void CommsBundleField::commsAddCustomReadRefreshOptInternal(StringsList& opts) const
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
        util::addToStrList("comms::option::def::HasCustomRead", opts);
    }

    if (hasGeneratedRefresh) {
        util::addToStrList("comms::option::def::HasCustomRefresh", opts);
    }
}

} // namespace commsdsl2new
