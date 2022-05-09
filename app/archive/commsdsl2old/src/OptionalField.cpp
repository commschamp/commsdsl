//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "OptionalField.h"

#include <type_traits>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2old
{

namespace
{

const std::string MembersDefTemplate =
    "/// @brief Scope for all the member fields of\n"
    "///     @ref #^#CLASS_NAME#$# optional.\n"
    "#^#EXTRA_PREFIX#$#\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#FIELD_DEF#$#\n"
    "};\n";

const std::string MembersOptionsTemplate =
    "/// @brief Extra options for all the member fields of\n"
    "///     @ref #^#SCOPE#$##^#CLASS_NAME#$# optional.\n"
    "struct #^#CLASS_NAME#$#Members#^#EXT#$#\n"
    "{\n"
    "    #^#OPTIONS#$#\n"
    "};\n";

const std::string ClassTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::Optional<\n"
    "        #^#FIELD_REF#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::Optional<\n"
    "            #^#FIELD_REF#$##^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#PUBLIC#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "#^#PROTECTED#$#\n"
    "#^#PRIVATE#$#\n"
    "};\n"
);

const std::string StructTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::Optional<\n"
    "        #^#FIELD_REF#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#NAME#$#\n"
    "};\n"
);

bool shouldUseStruct(const common::ReplacementMap& replacements)
{
    auto hasNoValue =
        [&replacements](const std::string& val)
        {
            auto iter = replacements.find(val);
            return (iter == replacements.end()) || iter->second.empty();
        };

    return
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH") &&
        hasNoValue("PUBLIC") &&
        hasNoValue("PRIVATE") &&
        hasNoValue("PROTECTED");
}

} // namespace

bool OptionalField::prepareImpl()
{
    auto obj = optionalFieldDslObj();
    auto field = obj.field();
    assert(field.valid());

    if (!field.externalRef().empty()) {
        return true;
    }

    auto ptr = create(generator(), field);
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

    m_field = std::move(ptr);
    return true;
}

void OptionalField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/Optional.h"
    };

    common::mergeIncludes(List, includes);

    if (m_field) {
        assert(m_field->externalRef().empty());
        m_field->updateIncludes(includes);
        return;
    }


    common::mergeInclude(generator().headerfileForField(optionalFieldDslObj().field().externalRef(), false), includes);
}

void OptionalField::updateIncludesCommonImpl(IncludesList& includes) const
{
    if (m_field) {
        assert(m_field->externalRef().empty());
        m_field->updateIncludesCommon(includes);
    }
}

void OptionalField::updatePluginIncludesImpl(Field::IncludesList& includes) const
{
    if (m_field) {
        m_field->updatePluginIncludes(includes);
        return;
    }

    common::mergeInclude(generator().headerfileForFieldInPlugin(optionalFieldDslObj().field().externalRef(), false), includes);
}

std::size_t OptionalField::maxLengthImpl() const
{
    auto* fieldPtr = getField();
    assert(fieldPtr != nullptr);
    return fieldPtr->maxLength();
}

std::string OptionalField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    if (!m_field) {
        // Find to mark it as used
        auto* foundField = generator().findField(optionalFieldDslObj().field().externalRef());
        if (foundField == nullptr) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
        }
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameCommonWrapFunc(adjustScopeWithNamespace(scope))));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("FIELD_REF", getFieldRef()));
    replacements.insert(std::make_pair("MEMBERS_STRUCT_DEF", getMembersDef(scope)));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["COMMA"] = ',';
    }

    if (!externalRef().empty()) {
        replacements.insert(std::make_pair("MEMBERS_OPT", "<TOpt>"));
    }

    auto* templ = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templ = &StructTemplate;
    }
    return common::processTemplate(*templ, replacements);
}

std::string OptionalField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getDefaultOptions, std::string());
}

std::string OptionalField::getExtraBareMetalDefaultOptionsImpl(const std::string& base, const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getBareMetalDefaultOptions, base);
}

std::string OptionalField::getExtraDataViewDefaultOptionsImpl(const std::string& base, const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getDataViewDefaultOptions, base);
}

std::string OptionalField::getPluginAnonNamespaceImpl(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    if (!m_field) {
        return common::emptyString();
    }

    auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr();
    if (!externalRef().empty()) {
        fullScope += "<>";
    }
    fullScope += "::";

    auto prop = m_field->getPluginCreatePropsFunc(fullScope, forcedSerialisedHidden, serHiddenParam);

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#PROP#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROP", std::move(prop)));
    return common::processTemplate(Templ, replacements);
}

std::string OptionalField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    common::StringsList options;
    auto obj = optionalFieldDslObj();
    if (obj.cond().valid() || obj.externalModeCtrl()) {
        options.push_back(".uncheckable()");
    }

    if (m_field) {
        auto prefix =
            common::nameToClassCopy(name()) + common::membersSuffixStr() +
            "::createProps_";

        auto str = ".field(" + prefix + common::nameToAccessCopy(m_field->name()) + "(";
        if (serHiddenParam) {
            str += common::serHiddenStr();
        }
        str +="))";
        options.push_back(std::move(str));
        return common::listToString(options, "\n", common::emptyString());
    }

    auto field = optionalFieldDslObj().field();
    auto extRef = field.externalRef();
    auto& name = field.name();
    auto dispName = common::displayName(field.displayName(), name);

    auto str =
        ".field(" +
        generator().scopeForFieldInPlugin(extRef) +
        "createProps_" + common::nameToAccessCopy(name) + "(\"" +
        dispName + "\"";
    if (serHiddenParam) {
        str += ", " + common::serHiddenStr();
    }
    str += "))";
    options.push_back(std::move(str));
    return common::listToString(options, "\n", common::emptyString());
}

std::string OptionalField::getPrivateRefreshBodyImpl(const Field::FieldsList& fields) const
{
    auto c = cond();
    if (!c.valid()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "auto mode = comms::field::OptionalMode::Missing;\n"
        "if (#^#COND#$#) {\n"
        "    mode = comms::field::OptionalMode::Exists;\n"
        "}\n\n"
        "if (field_#^#NAME#$#()#^#FIELD_ACC#$#.getMode() == mode) {\n"
        "    return false;\n"
        "}\n\n"
        "field_#^#NAME#$#()#^#FIELD_ACC#$#.setMode(mode);\n"
        "return true;\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(name())));
    replacements.insert(std::make_pair("COND", dslCondToString(fields, c)));

    if (isVersionOptional()) {
        replacements.insert(std::make_pair("FIELD_ACC", ".field()"));
    }

    return common::processTemplate(Templ, replacements);
}

bool OptionalField::hasCustomReadRefreshImpl() const
{
    return cond().valid();
}

std::string OptionalField::getReadPreparationImpl(const FieldsList& fields) const
{
    static_cast<void>(fields);
    return "refresh_" + common::nameToAccessCopy(name()) + "();\n";
}

bool OptionalField::isVersionDependentImpl() const
{
    return m_field && m_field->isVersionDependent();
}

std::string OptionalField::getCommonDefinitionImpl(const std::string& fullScope) const
{
    std::string membersCommon;
    do {
        if (!m_field) {
            break;
        }

        auto updatedScope = fullScope + common::membersSuffixStr() + "::";
        auto str = m_field->getCommonDefinition(updatedScope);
        if (str.empty()) {
            break;
        }

        static const std::string Templ =
            "/// @brief Scope for all the common definitions of the member fields of\n"
            "///     @ref #^#SCOPE#$# field.\n"
            "struct #^#CLASS_NAME#$#MembersCommon\n"
            "{\n"
            "    #^#DEFS#$#\n"
            "};\n";

        common::ReplacementMap repl;
        repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
        repl.insert(std::make_pair("SCOPE", fullScope));
        repl.insert(std::make_pair("DEFS", std::move(str)));
        membersCommon = common::processTemplate(Templ, repl);
    } while (false);

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

std::string OptionalField::getExtraRefToCommonDefinitionImpl(const std::string& fullScope) const
{
    if (!m_field) {
        return common::emptyString();
    }

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

std::string OptionalField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);
    checkModeOpt(options);

    return common::listToString(options, ",\n", common::emptyString());
}

std::string OptionalField::getMembersDef(const std::string& scope) const
{
    if (!m_field) {
        return common::emptyString();
    }

    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    std::string fieldDef = m_field->getClassDefinition(memberScope);

    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().scopeForOptions(common::defaultOptionsStr(), true, true) + ">";
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("FIELD_DEF", std::move(fieldDef)));
    return common::processTemplate(MembersDefTemplate, replacements);

}

std::string OptionalField::getFieldRef() const
{
    if (!m_field) {
        return generator().scopeForField(optionalFieldDslObj().field().externalRef(), true, true) + "<TOpt>";
    }

    std::string extraOpt;
    if (!externalRef().empty()) {
        extraOpt = "<TOpt>";
    }

    return
        "typename " + common::nameToClassCopy(name()) + common::membersSuffixStr() +
        extraOpt + "::" + common::nameToClassCopy(m_field->name());
}

void OptionalField::checkModeOpt(OptionalField::StringsList& options) const
{
    static const std::string Map[] = {
        common::emptyString(),
        "comms::option::def::ExistsByDefault",
        "comms::option::def::MissingByDefault"
    };

    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;

    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::OptionalField::Mode::NumOfValues), "Invalid map");

    auto obj = optionalFieldDslObj();
    auto mode = obj.defaultMode();
    auto idx = static_cast<std::size_t>(mode);
    if (MapSize <= idx) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        idx = 0U;
    }

    if (Map[idx].empty()) {
        return;
    }

    options.push_back(Map[idx]);
}

std::string OptionalField::getExtraOptions(const std::string& scope, GetExtraOptionsFunc func, const std::string& base) const
{
    if (!m_field) {
        return common::emptyString();
    }

    std::string nextBase;
    std::string ext;
    if (!base.empty()) {
        nextBase = base + "::" + common::nameToClassCopy(name()) + common::membersSuffixStr();
        ext = " : public " + nextBase;
    }    

    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    auto fieldOptions = (m_field.get()->*func)(nextBase, memberScope);

    if (fieldOptions.empty()) {
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("EXT", std::move(ext)));
    replacements.insert(std::make_pair("OPTIONS", std::move(fieldOptions)));
    return common::processTemplate(MembersOptionsTemplate, replacements);
}

const Field* OptionalField::getField() const
{
    if (m_field) {
        return m_field.get();
    }

    return generator().findField(optionalFieldDslObj().field().externalRef());
}

} // namespace commsdsl2old