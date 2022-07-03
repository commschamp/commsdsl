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

#include "CommsVariantField.h"

#include "CommsBundleField.h"
#include "CommsGenerator.h"
#include "CommsIntField.h"

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

namespace 
{

constexpr std::size_t MaxMembersSupportedByComms = 120;    

bool intIsValidPropKeyInternal(const CommsIntField& intField)
{
    auto obj = intField.field().dslObj();
    if (!obj.isFailOnInvalid()) {
        return false;
    }

    if (obj.isPseudo()) {
        return false;
    }

    auto intDslObj = commsdsl::parse::IntField(obj);
    auto& validRanges = intDslObj.validRanges();
    if (validRanges.size() != 1U) {
        return false;
    }

    auto& r = validRanges.front();
    if (r.m_min != r.m_max) {
        return false;
    }

    if (r.m_min != intDslObj.defaultValue()) {
        return false;
    }

    return true; 
}    

bool bundleStartsWithValidPropKeyInternal(const CommsBundleField& bundle)
{
    auto& members = bundle.commsMembers();
    if (members.empty()) {
        return false;
    }

    auto& first = members.front();
    if (first->field().dslObj().kind() != commsdsl::parse::Field::Kind::Int) {
        return false;
    }

    auto& keyField = static_cast<const CommsIntField&>(*first);
    if (!intIsValidPropKeyInternal(keyField)) {
        return false;
    }

    // Valid only if there is no non-default read
    return !keyField.commsHasGeneratedReadCode();
}

std::string bundlePropKeyTypeInternal(const CommsBundleField& bundle)
{
    auto& members = bundle.commsMembers();
    assert(!members.empty());

    auto& first = members.front();
    assert(first->field().dslObj().kind() == commsdsl::parse::Field::Kind::Int);

    auto& keyField = static_cast<const CommsIntField&>(*first);
    return keyField.commsVariantPropKeyType();
}

} // namespace 
    

CommsVariantField::CommsVariantField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsVariantField::prepareImpl()
{
    return 
        Base::prepareImpl() && 
        commsPrepare() &&
        commsPrepareInternal();
}

bool CommsVariantField::writeImpl() const
{
    return commsWrite();
}

CommsVariantField::IncludesList CommsVariantField::commsCommonIncludesImpl() const
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

std::string CommsVariantField::commsCommonCodeBodyImpl() const
{
    return commsCommonNameFuncCode();
}

std::string CommsVariantField::commsCommonMembersCodeImpl() const
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

CommsVariantField::IncludesList CommsVariantField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/CompileControl.h",
        "comms/field/Variant.h",
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

std::string CommsVariantField::commsDefMembersCodeImpl() const
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

std::string CommsVariantField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::Variant<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
        "    typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = generator();
    auto dslObj = variantDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.schemaOf(*this).mainNamespace()},
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

std::string CommsVariantField::commsDefPublicCodeImpl() const
{
    return commsDefAccessCodeInternal() + '\n' + commsDefFieldExecCodeInternal();
}

std::string CommsVariantField::commsDefPrivateCodeImpl() const
{
    static const std::string Templ = 
        "template <std::size_t TIdx, typename TField, typename TFunc>\n"
        "static void memFieldDispatch(TField&& f, TFunc&& func)\n"
        "{\n"
        "    #ifdef _MSC_VER\n"
        "        func.operator()<TIdx>(std::forward<TField>(f)); // VS compiler\n"
        "    #else // #ifdef _MSC_VER\n"
        "        func.template operator()<TIdx>(std::forward<TField>(f)); // All other compilers\n"
        "    #endif // #ifdef _MSC_VER\n"
        "}\n";
    return Templ;
}

std::string CommsVariantField::commsDefReadFuncBodyImpl() const
{
    if (m_optimizedReadKey.empty()) {
        return strings::emptyString();
    }

    std::string keyFieldType;
    StringsList cases;
    bool hasDefault = false;
    for (auto* m : m_members) {
        assert(m->field().dslObj().kind() == commsdsl::parse::Field::Kind::Bundle);
        auto& bundle = static_cast<const CommsBundleField&>(*m);
        auto& bundleMembers = bundle.commsMembers();
        assert(!bundleMembers.empty());
        auto* first = bundleMembers.front();
        assert(first->field().dslObj().kind() == commsdsl::parse::Field::Kind::Int);
        auto& keyField = static_cast<const CommsIntField&>(*first);
        auto bundleAccName = comms::accessName(bundle.field().dslObj().name());
        auto keyAccName = comms::accessName(keyField.field().dslObj().name());

        if ((m != m_members.back()) && 
            (keyField.commsVariantIsValidPropKey()) && 
            (keyField.commsVariantPropKeyType() == m_optimizedReadKey)) {
            auto valStr = keyField.commsVariantPropKeyValueStr();

            static const std::string Templ =
                "case #^#VAL#$#:\n"
                "    {\n"
                "        auto& field_#^#BUNDLE_NAME#$# = initField_#^#BUNDLE_NAME#$#();\n"
                "        COMMS_ASSERT(field_#^#BUNDLE_NAME#$#.field_#^#KEY_NAME#$#().getValue() == commonKeyField.getValue());\n"
                "        #^#VERSION_ASSIGN#$#\n"
                "        return field_#^#BUNDLE_NAME#$#.template readFrom<1>(iter, len);\n"
                "    }";

            util::ReplacementMap repl = {
                {"VAL", std::move(valStr)},
                {"BUNDLE_NAME", bundleAccName},
                {"KEY_NAME", keyAccName},
            };

            if (m->commsIsVersionDependent()) {
                auto assignStr = "field_" + bundleAccName + ".setVersion(Base::getVersion());";
                repl["VERSION_ASSIGN"] = std::move(assignStr);
            }
            cases.push_back(util::processTemplate(Templ, repl));
            continue;
        }

        // Last "catch all" element
        assert(m == m_members.back());

        static const std::string Templ =
            "default:\n"
            "    initField_#^#BUNDLE_NAME#$#().field_#^#KEY_NAME#$#().setValue(commonKeyField.getValue());\n"
            "    #^#VERSION_ASSIGN#$#\n"
            "    return accessField_#^#BUNDLE_NAME#$#().template readFrom<1>(iter, len);";

        util::ReplacementMap repl = {
            {"BUNDLE_NAME", bundleAccName},
            {"KEY_NAME", keyAccName},
        };

        if (m->commsIsVersionDependent()) {
            auto assignStr = "field_" + bundleAccName + ".setVersion(Base::getVersion());";
            repl["VERSION_ASSIGN"] = std::move(assignStr);
        }

        cases.push_back(util::processTemplate(Templ, repl));
        hasDefault = true;
    }

    if (!hasDefault) {
        static const std::string DefaultBreakStr =
            "default:\n"
            "    break;";
        cases.push_back(DefaultBreakStr);
    }

    static const std::string Templ =
        "#^#VERSION_DEP#$#\n"
        "using CommonKeyField=\n"
        "    #^#KEY_FIELD_TYPE#$#;\n"
        "CommonKeyField commonKeyField;\n\n"
        "auto origIter = iter;\n"
        "auto es = commonKeyField.read(iter, len);\n"
        "if (es != comms::ErrorStatus::Success) {\n"
        "    return es;\n"
        "}\n\n"
        "auto consumedLen = static_cast<std::size_t>(std::distance(origIter, iter));\n"
        "COMMS_ASSERT(consumedLen <= len);\n"
        "len -= consumedLen;\n\n"
        "switch (commonKeyField.getValue()) {\n"
        "#^#CASES#$#\n"
        "};\n\n"
        "return comms::ErrorStatus::InvalidMsgData;\n";

    util::ReplacementMap repl = {
        {"KEY_FIELD_TYPE", m_optimizedReadKey},
        {"CASES", util::strListToString(cases, "\n", "")},
    };

    if (commsIsVersionDependent()) {
        static const std::string CheckStr =
            "static_assert(Base::isVersionDependent(), \"The field must be recognised as version dependent\");";
        repl["VERSION_DEP"] = CheckStr;
    }

    return util::processTemplate(Templ, repl);    
}

CommsVariantField::StringsList CommsVariantField::commsDefReadMsvcSuppressWarningsImpl() const
{
    return StringsList{"4702"};
}

bool CommsVariantField::commsIsVersionDependentImpl() const
{
    return 
        std::any_of(
            m_members.begin(), m_members.end(),
            [](auto* m)
            {
                return m->commsIsVersionDependent();
            });
}

std::string CommsVariantField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
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

std::size_t CommsVariantField::commsMaxLengthImpl() const 
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), std::size_t(0),
            [](std::size_t soFar, auto* m)
            {
                return std::max(soFar, m->commsMaxLength());
            });
}

bool CommsVariantField::commsPrepareInternal()
{
    m_members = commsTransformFieldsList(members());
    if (generator().schemaOf(*this).versionDependentCode()) {
        auto sinceVersion = dslObj().sinceVersion();
        for (auto* m : m_members) {
            assert(m != nullptr);
            if (sinceVersion < m->field().dslObj().sinceVersion()) {
                generator().logger().error("Currently version dependent members of variant are not supported!");
                return false;
            }
        }
    }

    m_optimizedReadKey = commsOptimizedReadKeyInternal();
    return true;
}

std::string CommsVariantField::commsDefFieldOptsInternal() const
{
    commsdsl::gen::util::StringsList opts;
    commsAddFieldDefOptions(opts);
    commsAddDefaultIdxOptInternal(opts);
    commsAddCustomReadOptInternal(opts);
    return util::strListToString(opts, ",\n", "");
}

std::string CommsVariantField::commsDefAccessCodeInternal() const
{
    if (m_members.size() <= MaxMembersSupportedByComms) {
        return commsDefAccessCodeByCommsInternal();
    }

    return commsDefAccessCodeGeneratedInternal();
}

std::string CommsVariantField::commsDefAccessCodeByCommsInternal() const
{
    static const std::string Templ =
        "/// @brief Allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_VARIANT_MEMBERS_NAMES macro\n"
        "///     related to @b comms::field::Variant class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated values, types and access functions are:\n"
        "#^#ACCESS_DOC#$#\n"
        "COMMS_VARIANT_MEMBERS_NAMES(\n"
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
            "///     @li @b FieldIdx_" +  namesList.back() + 
            " index, @b Field_" + namesList.back() + " type,\n"
            "///         @b initField_" + namesList.back() +
            "() and @b accessField_" + namesList.back() + "() access functions -\n"
            "///         for " + comms::scopeFor(*mPtr, gen) + " member field.";
        accessDocList.push_back(std::move(accessStr));
    }

    util::ReplacementMap repl = {
        {"ACCESS_DOC", util::strListToString(accessDocList, "\n", "")},
        {"NAMES", util::strListToString(namesList, ",\n", "")},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefAccessCodeGeneratedInternal() const
{
    StringsList indicesList;
    StringsList accessList;
    indicesList.reserve(m_members.size());
    accessList.reserve(m_members.size());

    auto membersPrefix = comms::className(dslObj().name()) + strings::membersSuffixStr();
    auto docScope = membersPrefix + "::";
    auto typeScope = "typename " + membersPrefix + "<TOpt>::";
    for (auto& m : m_members) {
        auto accName = comms::accessName(m->field().dslObj().name());
        auto className = comms::className(m->field().dslObj().name());

        indicesList.push_back("FieldIdx_" + accName);

        static const std::string AccTempl =
            "/// @brief Member type alias to #^#DOC_SCOPE#$#.\n"
            "using Field_#^#NAME#$# = #^#TYPE_SCOPE#$#;\n\n"
            "/// @brief Initialize as #^#DOC_SCOPE#$#\n"
            "Field_#^#NAME#$#& initField_#^#NAME#$#()\n"
            "{\n"
            "    return Base::template initField<FieldIdx_#^#NAME#$#>();\n"
            "}\n\n"    
            "/// @brief Access as #^#DOC_SCOPE#$#\n"
            "Field_#^#NAME#$#& accessField_#^#NAME#$#()\n"
            "{\n"
            "    return Base::template accessField<FieldIdx_#^#NAME#$#>();\n"
            "}\n\n"
            "/// @brief Access as #^#DOC_SCOPE#$# (const version)\n"
            "const Field_#^#NAME#$#& accessField_#^#NAME#$#() const\n"
            "{\n"
            "    return Base::template accessField<FieldIdx_#^#NAME#$#>();\n"
            "}\n\n";   

        util::ReplacementMap accRepl = {
            {"DOC_SCOPE", docScope + className},
            {"TYPE_SCOPE", typeScope + className},
            {"NAME", accName}
        };
        accessList.push_back(util::processTemplate(AccTempl, accRepl));
    }

    static const std::string Templ =
        "/// @brief Allow access to internal fields by index.\n"
        "enum FieldIdx : unsigned\n"
        "{\n"
        "    #^#INDICES#$#,\n"
        "    FieldIdx_numOfValues"
        "};\n\n"
        "#^#ACCESS#$#\n";    

    util::ReplacementMap repl = {
        {"INDICES", util::strListToString(indicesList, ",\n", "")},
        {"ACCESS", util::strListToString(accessList, "\n", "")},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsVariantField::commsDefFieldExecCodeInternal() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_members.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#:\n"
            "    memFieldDispatch<FieldIdx_#^#MEM_NAME#$#>(accessField_#^#MEM_NAME#$#(), std::forward<TFunc>(func));\n"
            "    break;";
        util::ReplacementMap repl = {
            {"IDX", util::numToString(idx)},
            {"MEM_NAME", comms::accessName(m_members[idx]->field().dslObj().name())}
        };
        cases.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ =
        "/// @brief Optimized currFieldExec functionality#^#VARIANT#$#.\n"
        "/// @details Replaces the currFieldExec() member function defined\n"
        "///    by @b comms::field::Variant.\n"
        "template <typename TFunc>\n"
        "void currFieldExec(TFunc&& func) #^#CONST#$#\n"
        "{\n"
        "    switch (Base::currentField()) {\n"
        "    #^#CASES#$#\n"
        "    default:\n"
        "        static constexpr bool Invalid_field_execution = false;\n"
        "        static_cast<void>(Invalid_field_execution);\n"
        "        COMMS_ASSERT(Invalid_field_execution);\n"
        "        break;\n"
        "    }\n"
        "}\n";

    util::ReplacementMap repl = {
        {"CASES", util::strListToString(cases, "\n", "")}
    };

    auto str = util::processTemplate(Templ, repl);
    str += "\n";

    repl.insert({
        {"VARIANT", " (const variant)"},
        {"CONST", "const"}
    });
    str += util::processTemplate(Templ, repl);
    return str;
}

void CommsVariantField::commsAddDefaultIdxOptInternal(StringsList& opts) const
{
    auto obj = variantDslObj();
    auto idx = obj.defaultMemberIdx();
    if (idx < m_members.size()) {
        auto str = "comms::option::def::DefaultVariantIndex<" + util::numToString(idx) + '>';
        opts.push_back(std::move(str));
    }
}

void CommsVariantField::commsAddCustomReadOptInternal(StringsList& opts) const
{
    if (!m_optimizedReadKey.empty()) {
        util::addToStrList("comms::option::def::HasCustomRead", opts);
    }
}

std::string CommsVariantField::commsOptimizedReadKeyInternal() const
{
    std::string result;
    if (m_members.size() <= 1U) {
        return result;
    }

    std::string propType;
    for (auto& m : m_members) {
        if (m->field().dslObj().kind() != commsdsl::parse::Field::Kind::Bundle) {
            return result;
        }

        auto& bundle = static_cast<const CommsBundleField&>(*m);
        bool validPropKey = bundleStartsWithValidPropKeyInternal(bundle);
        if ((!validPropKey) && (&m != &m_members.back())) {
            return result;
        }

        if (!validPropKey) {
            // last "catch all" element
            continue;
        }

        std::string propTypeTmp = bundlePropKeyTypeInternal(bundle);
        if (propTypeTmp.empty()) {
            return result;
        }

        if (propType.empty()) {
            propType = std::move(propTypeTmp);
            continue;
        }

        if (propTypeTmp != propType) {
            // Type is not the same between elements
            return result;
        }
    }

    result = propType;
    return result;
}

} // namespace commsdsl2comms
