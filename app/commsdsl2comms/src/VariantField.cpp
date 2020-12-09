//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "VariantField.h"

#include <type_traits>
#include <numeric>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "BundleField.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

constexpr std::size_t MaxMembersSupportedByComms = 120;

const std::string MembersDefTemplate =
    "/// @brief Scope for all the member fields of\n"
    "///     @ref #^#CLASS_NAME#$# bitfield.\n"
    "#^#EXTRA_PREFIX#$#\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#MEMBERS_DEFS#$#\n"
    "    /// @brief All members bundled in @b std::tuple.\n"
    "    using All =\n"
    "        std::tuple<\n"
    "           #^#MEMBERS#$#\n"
    "        >;\n"
    "};\n";

const std::string MembersOptionsTemplate =
    "/// @brief Extra options for all the member fields of\n"
    "///     @ref #^#SCOPE#$##^#CLASS_NAME#$# bitfield.\n"
    "struct #^#CLASS_NAME#$#Members#^#EXT#$#\n"
    "{\n"
    "    #^#OPTIONS#$#\n"
    "};\n";

const std::string ClassTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::Variant<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "        typename #^#ORIG_CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::Variant<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "            typename #^#ORIG_CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#ACCESS#$#\n"
    "    #^#PUBLIC#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "    #^#CURR_FIELD_EXEC#$#\n"
    "#^#PROTECTED#$#\n"
    "private:\n"
    "    template <std::size_t TIdx, typename TField, typename TFunc>\n"
    "    static void memFieldDispatch(TField&& f, TFunc&& func)\n"
    "    {\n"
    "        #ifdef _MSC_VER\n"
    "            func.operator()<TIdx>(std::forward<TField>(f)); // VS compiler\n"
    "        #else // #ifdef _MSC_VER\n"
    "            func.template operator()<TIdx>(std::forward<TField>(f)); // All other compilers\n"
    "        #endif // #ifdef _MSC_VER\n"
    "    }\n"
    "    #^#PRIVATE#$#\n"
    "};\n"
);

} // namespace

bool VariantField::prepareImpl()
{
    auto obj = variantFieldDslObj();
    auto members = obj.members();
    m_members.reserve(members.size());
    for (auto& m : members) {
        auto ptr = create(generator(), m);
        if (!ptr) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            return false;
        }

        ptr->setMemberChild();
        if (!ptr->prepare(obj.sinceVersion())) {
            return false;
        }

        if ((generator().versionDependentCode()) && 
            (obj.sinceVersion() < ptr->sinceVersion())) {
            generator().logger().error("Currently version dependent members of variant are not supported!");
            return false;
        }

        m_members.push_back(std::move(ptr));
    }
    return true;
}

void VariantField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/CompileControl.h",
        "comms/field/Variant.h",
        "<tuple>"
    };

    common::mergeIncludes(List, includes);

    for (auto& m : m_members) {
        m->updateIncludes(includes);
    }
}

void VariantField::updateIncludesCommonImpl(IncludesList& includes) const
{
    for (auto& m : m_members) {
        m->updateIncludesCommon(includes);
    }
}

void VariantField::updatePluginIncludesImpl(Field::IncludesList& includes) const
{
    for (auto& m : m_members) {
        m->updatePluginIncludes(includes);
    }
}

std::size_t VariantField::maxLengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), std::size_t(0),
            [](std::size_t soFar, auto& m)
            {
                return std::max(soFar, m->maxLength());
            });
}

std::string VariantField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("ORIG_CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameCommonWrapFunc(adjustScopeWithNamespace(scope))));
    replacements.insert(std::make_pair("READ", getRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getRefresh()));
    replacements.insert(std::make_pair("CURR_FIELD_EXEC", getCurrFieldExec()));
    replacements.insert(std::make_pair("MEMBERS_STRUCT_DEF", getMembersDef(scope)));
    replacements.insert(std::make_pair("ACCESS", getAccess()));
    replacements.insert(std::make_pair("PRIVATE", getPrivate()));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["COMMA"] = ',';
    }

    if (!externalRef().empty()) {
        replacements.insert(std::make_pair("MEMBERS_OPT", "<TOpt>"));
    }

    return common::processTemplate(ClassTemplate, replacements);
}

std::string VariantField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getDefaultOptions, std::string());
}

std::string VariantField::getExtraBareMetalDefaultOptionsImpl(const std::string& base, const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getBareMetalDefaultOptions, base);
}

std::string VariantField::getExtraDataViewDefaultOptionsImpl(const std::string& base, const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getDataViewDefaultOptions, base);
}

std::string VariantField::getPluginAnonNamespaceImpl(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr();
    if (!externalRef().empty()) {
        fullScope += "<>";
    }
    fullScope += "::";

    common::StringsList props;
    for (auto& f : m_members) {
        props.push_back(f->getPluginCreatePropsFunc(fullScope, forcedSerialisedHidden, serHiddenParam));
    }

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#PROPS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROPS", common::listToString(props, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string VariantField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    common::StringsList props;
    props.reserve(m_members.size() + 1);
    auto prefix =
        common::nameToClassCopy(name()) + common::membersSuffixStr() +
        "::createProps_";
    for (auto& f : m_members) {
        auto str = ".add(" + prefix + common::nameToAccessCopy(f->name()) + "(";
        if (serHiddenParam) {
            str += common::serHiddenStr();
        }
        str += "))";
        props.push_back(std::move(str));
    }

    auto obj = variantFieldDslObj();
    if (obj.displayIdxReadOnlyHidden()) {
        props.push_back(".setIndexHidden()");
    }

    props.push_back(".serialisedHidden()");

    return common::listToString(props, "\n", common::emptyString());
}

void VariantField::setForcedPseudoImpl()
{
    for (auto& m : m_members) {
        m->setForcedPseudo();
    }
}

void VariantField::setForcedNoOptionsConfigImpl()
{
    for (auto& m : m_members) {
        m->setForcedNoOptionsConfig();
    }
}

bool VariantField::isVersionDependentImpl() const
{
    return
        std::any_of(
            m_members.begin(), m_members.end(),
            [](auto& m)
            {
                return m->isVersionDependent();
            });
}

std::string VariantField::getCommonDefinitionImpl(const std::string& fullScope) const
{
    common::StringsList defs;
    auto updatedScope = fullScope + common::membersSuffixStr() + "::";
    for (auto& m : m_members) {
        auto str = m->getCommonDefinition(updatedScope);
        if (!str.empty()) {
            defs.emplace_back(std::move(str));
        }
    }

    std::string membersCommon;
    if (!defs.empty()) {
        static const std::string Templ =
            "/// @brief Scope for all the common definitions of the member fields of\n"
            "///     @ref #^#SCOPE#$# bundle.\n"
            "struct #^#CLASS_NAME#$#MembersCommon\n"
            "{\n"
            "    #^#DEFS#$#\n"
            "};\n";

        common::ReplacementMap repl;
        repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
        repl.insert(std::make_pair("SCOPE", fullScope));
        repl.insert(std::make_pair("DEFS", common::listToString(defs, "\n", common::emptyString())));
        membersCommon = common::processTemplate(Templ, repl);
    }

    static const std::string Templ =
        "#^#COMMON#$#\n"
        "/// @brief Scope for all the common definitions of the\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "struct #^#CLASS_NAME#$#Common\n"
        "{\n"
        "    #^#NAME_FUNC#$#\n"
        "};\n\n";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("COMMON", std::move(membersCommon)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    repl.insert(std::make_pair("SCOPE", fullScope));
    repl.insert(std::make_pair("NAME_FUNC", getCommonNameFunc(fullScope)));
    return common::processTemplate(Templ, repl);
}

std::string VariantField::getExtraRefToCommonDefinitionImpl(const std::string& fullScope) const
{
    static const std::string Templ =
        "/// @brief Common types and functions for members of\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "using #^#CLASS_NAME#$#MembersCommon = #^#COMMON_SCOPE#$#MembersCommon;\n\n";

    auto commonScope = scopeForCommon(generator().scopeForField(externalRef(), true, true));
    std::string className = classNameFromFullScope(fullScope);

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", fullScope));
    repl.insert(std::make_pair("CLASS_NAME", std::move(className)));
    repl.insert(std::make_pair("COMMON_SCOPE", std::move(commonScope)));

    return common::processTemplate(Templ, repl);
}

std::string VariantField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    auto obj = variantFieldDslObj();
    auto idx = obj.defaultMemberIdx();
    if (idx < m_members.size()) {
        auto opt = "comms::option::def::DefaultVariantIndex<" + common::numToString(idx) + '>';
        common::addToList(opt, options);
    }

    if (hasOptimizedRead()) {
        common::addToList("comms::option::def::HasCustomRead", options);
    }

    return common::listToString(options, ",\n", common::emptyString());
}

std::string VariantField::getMembersDef(const std::string& scope) const
{
    auto className = common::nameToClassCopy(name());
    std::string memberScope;
    if (!scope.empty()) {
        memberScope = scope + className + common::membersSuffixStr() + "::";
    }
    StringsList membersDefs;
    StringsList membersNames;

    membersDefs.reserve(m_members.size());
    membersNames.reserve(m_members.size());
    for (auto& m : m_members) {
        membersDefs.push_back(m->getClassDefinition(memberScope));
        membersNames.push_back(common::nameToClassCopy(m->name()));
    }

    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().scopeForOptions(common::defaultOptionsStr(), true, true) + ">";
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("MEMBERS_DEFS", common::listToString(membersDefs, "\n", common::emptyString())));
    replacements.insert(std::make_pair("MEMBERS", common::listToString(membersNames, ",\n", common::emptyString())));
    return common::processTemplate(MembersDefTemplate, replacements);

}

std::string VariantField::getAccess() const
{
    if (m_members.size() <= MaxMembersSupportedByComms) {
        return getAccessByComms();
    }

    return getAccessGenerated();
}

std::string VariantField::getAccessByComms() const
{
    static const std::string Templ =
        "/// @brief Allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_VARIANT_MEMBERS_NAMES macro\n"
        "///     related to @b comms::field::Variant class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated types and access functions are:\n"
        "#^#ACCESS_DOC#$#\n"
        "COMMS_VARIANT_MEMBERS_NAMES(\n"
        "    #^#NAMES#$#\n"
        ");\n";

    StringsList accessDocList;
    StringsList namesList;
    accessDocList.reserve(m_members.size());
    namesList.reserve(m_members.size());

    auto scope = common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    for (auto& m : m_members) {
        namesList.push_back(common::nameToAccessCopy(m->name()));
        std::string accessStr =
            "///     @li @b Field_" + namesList.back() + " type, @b initField_" + namesList.back() +
            "() and @b accessField_" + namesList.back() + "() access functions -\n"
            "///     for " + scope +
            common::nameToClassCopy(m->name()) + " member field.";
        accessDocList.push_back(std::move(accessStr));
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ACCESS_DOC", common::listToString(accessDocList, "\n", common::emptyString())));
    replacements.insert(std::make_pair("NAMES", common::listToString(namesList, ",\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string VariantField::getAccessGenerated() const
{
    static const std::string Templ =
        "/// @brief Allow access to internal fields by index.\n"
        "enum FieldIdx : unsigned\n"
        "{\n"
            "#^#INDICES#$#,\n"
            "FieldIdx_numOfValues"
        "};\n\n"
        "#^#ACCESS#$#\n";

    StringsList indicesList;
    StringsList accessList;
    indicesList.reserve(m_members.size());
    accessList.reserve(m_members.size());

    auto docScope = common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    auto typeScope = "typename " + common::nameToClassCopy(name()) + common::membersSuffixStr() + "<TOpt>::";
    for (auto& m : m_members) {
        auto accName = common::nameToAccessCopy(m->name());
        auto className = common::nameToClassCopy(m->name());

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

        common::ReplacementMap accRepl;
        accRepl.insert(std::make_pair("DOC_SCOPE", docScope + className));
        accRepl.insert(std::make_pair("TYPE_SCOPE", typeScope + className));
        accRepl.insert(std::make_pair("NAME", accName));
        accessList.push_back(common::processTemplate(AccTempl, accRepl));
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("INDICES", common::listToString(indicesList, ",\n", common::emptyString())));
    replacements.insert(std::make_pair("ACCESS", common::listToString(accessList, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string VariantField::getRead() const
{
    auto customRead = getCustomRead();
    if (!customRead.empty()) {
        return customRead;
    }

    if (!hasOptimizedRead()) {
        return common::emptyString();
    }

    std::string keyFieldType;
    StringsList cases;
    bool hasDefault = false;
    for (auto& m : m_members) {
        assert(m->kind() == commsdsl::Field::Kind::Bundle);

        auto& bundle = static_cast<const BundleField&>(*m);

        if (keyFieldType.empty()) {
            assert(bundle.startsWithValidPropKey());
            keyFieldType = bundle.getPropKeyType();
        }

        auto propKeyName = bundle.getFirstMemberName();
        if (propKeyName.empty()) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            return common::emptyString();
        }

        if (bundle.startsWithValidPropKey()) {
            auto valStr = bundle.getPropKeyValueStr();
            if (valStr.empty()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
                assert(Should_not_happen);
                return common::emptyString();
            }

            auto valHexStr = bundle.getPropKeyValueStr(true);
            if (!valHexStr.empty()) {
                valHexStr = " // " + valHexStr;
            }

            static const std::string Templ =
                "case #^#VAL#$#:#^#VAL_HEX#$#\n"
                "    {\n"
                "        auto& field_#^#BUNDLE_NAME#$# = initField_#^#BUNDLE_NAME#$#();\n"
                "        COMMS_ASSERT(field_#^#BUNDLE_NAME#$#.field_#^#KEY_NAME#$#().value() == commonKeyField.value());\n"
                "        #^#VERSION_ASSIGN#$#\n"
                "        return field_#^#BUNDLE_NAME#$#.template readFrom<1>(iter, len);\n"
                "    }";

            common::ReplacementMap repl;
            repl.insert(std::make_pair("VAL", std::move(valStr)));
            repl.insert(std::make_pair("VAL_HEX", std::move(valHexStr)));
            repl.insert(std::make_pair("BUNDLE_NAME", common::nameToAccessCopy(bundle.name())));
            repl.insert(std::make_pair("KEY_NAME", common::nameToAccessCopy(propKeyName)));

            if (m->isVersionDependent()) {
                auto assignStr =
                    "field_" + common::nameToAccessCopy(bundle.name()) +
                    ".setVersion(Base::getVersion());";
                repl.insert(std::make_pair("VERSION_ASSIGN", std::move(assignStr)));
            }
            cases.push_back(common::processTemplate(Templ, repl));
            continue;
        }

        // Last "catch all" element
        assert(&m == &m_members.back());

        static const std::string Templ =
            "default:\n"
            "    initField_#^#BUNDLE_NAME#$#().field_#^#KEY_NAME#$#().value() = commonKeyField.value();\n"
            "    #^#VERSION_ASSIGN#$#\n"
            "    return accessField_#^#BUNDLE_NAME#$#().template readFrom<1>(iter, len);";

        common::ReplacementMap repl;
        repl.insert(std::make_pair("BUNDLE_NAME", common::nameToAccessCopy(bundle.name())));
        repl.insert(std::make_pair("KEY_NAME", common::nameToAccessCopy(propKeyName)));

        if (m->isVersionDependent()) {
            auto assignStr =
                "accessField_" + common::nameToAccessCopy(bundle.name()) +
                "().setVersion(Base::getVersion());";
            repl.insert(std::make_pair("VERSION_ASSIGN", std::move(assignStr)));
        }

        cases.push_back(common::processTemplate(Templ, repl));
        hasDefault = true;
    }

    if (!hasDefault) {
        static const std::string DefaultBreakStr =
            "default:\n"
            "    break;";
        cases.push_back(DefaultBreakStr);
    }

    auto casesStr = common::listToString(cases, "\n", common::emptyString());

     common::ReplacementMap repl;
     repl.insert(std::make_pair("KEY_FIELD_TYPE", std::move(keyFieldType)));
     repl.insert(std::make_pair("CASES", std::move(casesStr)));

     if (isVersionDependent()) {
        static const std::string CheckStr =
            "static_assert(Base::isVersionDependent(), \"The field must be recognised as version dependent\");";
        repl.insert(std::make_pair("VERSION_DEP", CheckStr));
     }

     static const std::string Templ =
        "COMMS_MSVC_WARNING_PUSH\n"
        "COMMS_MSVC_WARNING_DISABLE(4702) // Disable unreachable code warning\n"
        "/// @brief Optimized read functionality.\n"
        "template <typename TIter>\n"
        "comms::ErrorStatus read(TIter& iter, std::size_t len)\n"
        "{\n"
        "    #^#VERSION_DEP#$#\n"
        "    using CommonKeyField=\n"
        "        #^#KEY_FIELD_TYPE#$#;\n"
        "    CommonKeyField commonKeyField;\n\n"
        "    auto origIter = iter;\n"
        "    auto es = commonKeyField.read(iter, len);\n"
        "    if (es != comms::ErrorStatus::Success) {\n"
        "        return es;\n"
        "    }\n\n"
        "    auto consumedLen = static_cast<std::size_t>(std::distance(origIter, iter));\n"
        "    COMMS_ASSERT(consumedLen <= len);\n"
        "    len -= consumedLen;\n\n"
        "    switch (commonKeyField.value()) {\n"
        "    #^#CASES#$#\n"
        "    };\n\n"
        "    return comms::ErrorStatus::InvalidMsgData;\n"
        "}\n"
        "COMMS_MSVC_WARNING_POP\n";

     return common::processTemplate(Templ, repl);
}

std::string VariantField::getRefresh() const
{
    return getCustomRefresh();
}

std::string VariantField::getCurrFieldExec() const
{
    StringsList cases;
    for (auto idx = 0U; idx < m_members.size(); ++idx) {
        static const std::string Templ =
            "case FieldIdx_#^#MEM_NAME#$#:\n"
            "    memFieldDispatch<FieldIdx_#^#MEM_NAME#$#>(accessField_#^#MEM_NAME#$#(), std::forward<TFunc>(func));\n"
            "    break;";
        common::ReplacementMap repl;
        repl.insert(std::make_pair("IDX", common::numToString(idx)));
        repl.insert(std::make_pair("MEM_NAME", common::nameToAccessCopy(m_members[idx]->name())));
        cases.push_back(common::processTemplate(Templ, repl));
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

    common::ReplacementMap repl;
    repl.insert(std::make_pair("CASES", common::listToString(cases, "\n", common::emptyString())));
    auto str = common::processTemplate(Templ, repl);
    str += "\n";
    repl.insert(std::make_pair("VARIANT", " (const variant)"));
    repl.insert(std::make_pair("CONST", "const"));
    str += common::processTemplate(Templ, repl);
    return str;
}

std::string VariantField::getPrivate() const
{
    auto str = getExtraPrivate();
    auto refreshStr = getPrivateRefreshForFields(m_members);

    if ((!str.empty()) && (refreshStr.empty())) {
        str += '\n';
    }

    str += refreshStr;
    if (str.empty()) {
        return str;
    }

    return '\n' + str;
}

std::string VariantField::getExtraOptions(const std::string& scope, GetExtraOptionsFunc func, const std::string& base) const
{
    std::string nextBase;
    std::string ext;
    if (!base.empty()) {
        nextBase = base + "::" + common::nameToClassCopy(name()) + common::membersSuffixStr();
        ext = " : public " + nextBase;
    }

    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    StringsList options;
    options.reserve(m_members.size());
    for (auto& m : m_members) {
        auto opt = (m.get()->*func)(nextBase, memberScope);
        if (!opt.empty()) {
            options.push_back(std::move(opt));
        }
    }

    if (options.empty()) {
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("EXT", std::move(ext)));
    replacements.insert(std::make_pair("OPTIONS", common::listToString(options, "\n", common::emptyString())));
    return common::processTemplate(MembersOptionsTemplate, replacements);
}

bool VariantField::hasOptimizedRead() const
{
    if (m_members.size() <= 1U) {
        return false;
    }

    std::string propType;
    for (auto& m : m_members) {
        if (m->kind() != commsdsl::Field::Kind::Bundle) {
            return false;
        }

        auto& bundle = static_cast<const BundleField&>(*m);
        bool validPropKey = bundle.startsWithValidPropKey();
        if ((!validPropKey) && (&m != &m_members.back())) {
            return false;
        }

        if (!validPropKey) {
            // last "catch all" element
            continue;
        }

        std::string propTypeTmp = bundle.getPropKeyType();
        if (propTypeTmp.empty()) {
            return false;
        }

        if (propType.empty()) {
            propType = std::move(propTypeTmp);
            continue;
        }

        if (propTypeTmp != propType) {
            // Type is not the same between elements
            return false;
        }
    }

    assert(!propType.empty());
    return !propType.empty();
}

} // namespace commsdsl2comms
