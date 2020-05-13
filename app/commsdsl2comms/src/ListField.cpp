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

#include "ListField.h"

#include <type_traits>
#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#MEMBERS_DEF#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::ArrayList<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "        #^#ELEMENT#$##^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::ArrayList<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "            #^#ELEMENT#$##^#COMMA#$#\n"
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
    "#^#MEMBERS_DEF#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::ArrayList<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "        #^#ELEMENT#$##^#COMMA#$#\n"
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

bool ListField::prepareImpl()
{
    auto obj = listFieldDslObj();

    do {
        auto elementField = obj.elementField();
        assert(elementField.valid());
        if (!elementField.externalRef().empty()) {
            break;
        }

        m_element = create(generator(), elementField);
        m_element->setMemberChild();
        if (!m_element->prepare(dslObj().sinceVersion())) {
            return false;
        }
    } while (false);

    do {
        if (!obj.hasCountPrefixField()) {
            break;
        }

        auto prefix = obj.countPrefixField();
        if (!prefix.externalRef().empty()) {
            break;
        }

        m_countPrefix = create(generator(), prefix);
        m_countPrefix->setMemberChild();
        if (!m_countPrefix->prepare(dslObj().sinceVersion())) {
            return false;
        }

        if ((m_element) &&
            (common::nameToClassCopy(m_element->name()) == common::nameToClassCopy(m_countPrefix->name()))) {
            generator().logger().error("Count prefix and element fields of \"" + name() + "\" list must have different names.");
            return false;
        }

    } while (false);

    do {
        if (!obj.hasLengthPrefixField()) {
            break;
        }

        assert(!m_countPrefix);
        assert(!obj.hasCountPrefixField());
        auto prefix = obj.lengthPrefixField();
        if (!prefix.externalRef().empty()) {
            break;
        }

        m_lengthPrefix = create(generator(), prefix);
        m_lengthPrefix->setMemberChild();
        if (!m_lengthPrefix->prepare(dslObj().sinceVersion())) {
            return false;
        }

        if ((m_element) &&
            (common::nameToClassCopy(m_element->name()) == common::nameToClassCopy(m_lengthPrefix->name()))) {
            generator().logger().error("Length prefix and element fields of \"" + name() + "\" list must have different names.");
            return false;
        }

    } while (false);

    do {
        if (!obj.hasElemLengthPrefixField()) {
            break;
        }

        auto prefix = obj.elemLengthPrefixField();
        if (!prefix.externalRef().empty()) {
            break;
        }

        m_elemLengthPrefix = create(generator(), prefix);
        m_elemLengthPrefix->setMemberChild();
        if (!m_elemLengthPrefix->prepare(dslObj().sinceVersion())) {
            return false;
        }

        if ((m_element) &&
            (common::nameToClassCopy(m_element->name()) == common::nameToClassCopy(m_elemLengthPrefix->name()))) {
            generator().logger().error("Element length prefix and element fields of \"" + name() + "\" list must have different names.");
            return false;
        }

        if ((m_countPrefix) &&
            (common::nameToClassCopy(m_countPrefix->name()) == common::nameToClassCopy(m_elemLengthPrefix->name()))) {
            generator().logger().error("Element length prefix and count prefix fields of \"" + name() + "\" list must have different names.");
            return false;
        }

        if ((m_lengthPrefix) &&
            (common::nameToClassCopy(m_lengthPrefix->name()) == common::nameToClassCopy(m_elemLengthPrefix->name()))) {
            generator().logger().error("Element length prefix and list length prefix fields of \"" + name() + "\" list must have different names.");
            return false;
        }

    } while (false);

    return true;
}

void ListField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/ArrayList.h",
    };

    common::mergeIncludes(List, includes);

    auto obj = listFieldDslObj();
    do {
        if (m_element) {
            m_element->updateIncludes(includes);
            break;
        }

        auto elementField = obj.elementField();
        assert(elementField.valid());
        auto extRef = elementField.externalRef();
        assert(!extRef.empty());
        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);

    do {
        if (m_countPrefix) {
            m_countPrefix->updateIncludes(includes);
            break;
        }

        if (!obj.hasCountPrefixField()) {
            break;
        }

        auto extRef = obj.countPrefixField().externalRef();
        assert(!extRef.empty());

        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);

    do {
        if (m_lengthPrefix) {
            m_lengthPrefix->updateIncludes(includes);
            break;
        }

        if (!obj.hasLengthPrefixField()) {
            break;
        }

        auto extRef = obj.lengthPrefixField().externalRef();
        assert(!extRef.empty());

        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);

    do {
        if (m_elemLengthPrefix) {
            m_elemLengthPrefix->updateIncludes(includes);
            break;
        }

        if (!obj.hasElemLengthPrefixField()) {
            break;
        }

        auto prefixField = obj.elemLengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        common::mergeInclude(generator().headerfileForField(extRef, false), includes);
    } while (false);
}

void ListField::updateIncludesCommonImpl(IncludesList& includes) const
{
    auto updateFunc =
        [&includes](auto& f)
        {
            if (f) {
                f->updateIncludesCommon(includes);
            }
        };

    updateFunc(m_element);
    updateFunc(m_countPrefix);
    updateFunc(m_lengthPrefix);
    updateFunc(m_elemLengthPrefix);
}

void ListField::updatePluginIncludesImpl(Field::IncludesList& includes) const
{
    auto obj = listFieldDslObj();
    do {
        if (m_element) {
            m_element->updatePluginIncludes(includes);
            break;
        }

        auto elementField = obj.elementField();
        assert(elementField.valid());
        auto extRef = elementField.externalRef();
        assert(!extRef.empty());
        auto* elemFieldPtr = generator().findField(extRef, false);
        assert(elemFieldPtr != nullptr);
        elemFieldPtr->updatePluginIncludes(includes);
    } while (false);

    do {
        if (m_countPrefix) {
            m_countPrefix->updatePluginIncludes(includes);
            break;
        }

        if (!obj.hasCountPrefixField()) {
            break;
        }

        auto extRef = obj.countPrefixField().externalRef();
        assert(!extRef.empty());

        auto* countFieldPtr = generator().findField(extRef, false);
        assert(countFieldPtr != nullptr);
        countFieldPtr->updatePluginIncludes(includes);
    } while (false);

    do {
        if (m_lengthPrefix) {
            m_lengthPrefix->updatePluginIncludes(includes);
            break;
        }

        if (!obj.hasLengthPrefixField()) {
            break;
        }

        auto extRef = obj.lengthPrefixField().externalRef();
        assert(!extRef.empty());

        auto* lengthFieldPtr = generator().findField(extRef, false);
        assert(lengthFieldPtr != nullptr);
        lengthFieldPtr->updatePluginIncludes(includes);
    } while (false);

    do {
        if (m_elemLengthPrefix) {
            m_elemLengthPrefix->updatePluginIncludes(includes);
            break;
        }

        if (!obj.hasElemLengthPrefixField()) {
            break;
        }

        auto prefixField = obj.elemLengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        auto* elemLengthFieldPtr = generator().findField(extRef, false);
        assert(elemLengthFieldPtr != nullptr);
        elemLengthFieldPtr->updatePluginIncludes(includes);
    } while (false);
}

std::size_t ListField::maxLengthImpl() const
{
    auto obj = listFieldDslObj();
    if (obj.fixedCount() != 0U) {
        return Base::maxLengthImpl();
    }

    return common::maxPossibleLength();
}

std::string ListField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
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
    replacements.insert(std::make_pair("ELEMENT", getElement()));
    replacements.insert(std::make_pair("MEMBERS_DEF", getMembersDef(scope)));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));

    if (!replacements["FIELD_OPTS"].empty()) {
        replacements.insert(std::make_pair("COMMA", ","));
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string ListField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getDefaultOptions, std::string());
}

std::string ListField::getExtraBareMetalDefaultOptionsImpl(const std::string& base, const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getBareMetalDefaultOptions, base);
}

std::string ListField::getExtraDataViewDefaultOptionsImpl(const std::string& base, const std::string& scope) const
{
    return getExtraOptions(scope, &Field::getDataViewDefaultOptions, base);
}

std::string ListField::getBareMetalOptionStrImpl() const
{
    auto obj = listFieldDslObj();
    auto fixedCount = obj.fixedCount();
    if (fixedCount != 0U) {
        return "comms::option::app::SequenceFixedSizeUseFixedSizeStorage";
    }

    return "comms::option::app::FixedSizeStorage<" + common::seqDefaultSizeStr() + '>';
}

std::string ListField::getCompareToValueImpl(
    const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    static_cast<void>(op);
    static_cast<void>(value);
    static_cast<void>(nameOverride);
    static_cast<void>(forcedVersionOptional);
    assert(!"List field is not expected to be comparable");
    return common::emptyString();
}

std::string ListField::getCompareToFieldImpl(
    const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    static_cast<void>(op);
    static_cast<void>(field);
    static_cast<void>(nameOverride);
    static_cast<void>(forcedVersionOptional);
    assert(!"List field is not expected to be comparable");
    return common::emptyString();
}

std::string ListField::getPluginAnonNamespaceImpl(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    if (!m_element) {
        return common::emptyString();
    }

    auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr();
    if (!externalRef().empty()) {
        fullScope += "<>";
    }
    fullScope += "::";

    if (isElemForcedSerialisedHiddenInPlugin()) {
        forcedSerialisedHidden = true;
        serHiddenParam = false;
    }

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#PROPS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROPS", m_element->getPluginCreatePropsFunc(fullScope, forcedSerialisedHidden, serHiddenParam)));
    return common::processTemplate(Templ, replacements);
}

std::string ListField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    static_cast<void>(serHiddenParam);
    auto obj = listFieldDslObj();
    common::StringsList props;
    bool elemForcedSerHidden = isElemForcedSerialisedHiddenInPlugin();
    if (elemForcedSerHidden) {
        serHiddenParam = false;
    }

    if (m_element) {
        auto func =
            common::nameToClassCopy(name()) + common::membersSuffixStr() +
            "::createProps_" + common::nameToAccessCopy(m_element->name()) + "(";
        if (serHiddenParam) {
            func += common::serHiddenStr();
        }
        func += ")";
        props.push_back(".add(" + func + ")");
    }
    else {
        auto elemField = obj.elementField();
        assert(elemField.valid());
        auto extRef = elemField.externalRef();
        assert(!extRef.empty());
        auto scope = generator().scopeForFieldInPlugin(extRef);
        std::string fieldType("Field");
        if (isVersionOptional()) {
            fieldType = "InnerField";
        }
        auto func = scope + "createProps_" + common::nameToAccessCopy(elemField.name()) +
        "(" + fieldType + "::ValueType::value_type::name()";
        if (serHiddenParam) {
            func += ", " + common::serHiddenStr();
        }
        func += ")";
        props.push_back(".add(" + func + ")");
    }

    do {
        if (elemForcedSerHidden) {
            break;
        }

        props.push_back(".serialisedHidden()");
        if ((!obj.hasCountPrefixField()) && (!obj.hasLengthPrefixField())) {
            break;
        }

        props.push_back(".prefixName(\"" + getPrefixName() + "\")");
        props.push_back(".showPrefix()");
    } while (false);

    props.push_back(".appendIndexToElementName()");
    return common::listToString(props, "\n", common::emptyString());
}

std::string ListField::getPrivateRefreshBodyImpl(const FieldsList& fields) const
{
    auto obj = listFieldDslObj();

    common::StringsList refreshes;
    auto processPrefixFunc = 
        [&fields, &refreshes](const std::string& prefixName, const common::ReplacementMap& replacements)
        {
            if (prefixName.empty()) {
                return;
            }

            auto iter =
                std::find_if(
                    fields.begin(), fields.end(),
                    [&prefixName](auto& f)
                    {
                        return f->name() == prefixName;
                    });

            if (iter == fields.end()) {
                assert(!"Should not happen");
                return;
            }

            bool prefixVersionOptional = (*iter)->isVersionOptional();

            static const std::string Templ = 
                "do {\n"
                "    auto expectedValue = static_cast<std::size_t>(field_#^#PREFIX_NAME#$#()#^#PREFIX_ACC#$#.value());\n"
                "    #^#REAL_VALUE#$#\n"
                "    if (expectedValue != realValue) {\n"
                "        using PrefixValueType = typename std::decay<decltype(field_#^#PREFIX_NAME#$#()#^#PREFIX_ACC#$#.value())>::type;\n"
                "        field_#^#PREFIX_NAME#$#()#^#PREFIX_ACC#$#.value() = static_cast<PrefixValueType>(realValue);\n"
                "        updated = true;\n"
                "    }\n"
                "} while (false);\n";

            auto repl = replacements;
            repl.insert(std::make_pair("PREFIX_NAME", common::nameToAccessCopy(prefixName)));

            if (prefixVersionOptional) {
                repl.insert(std::make_pair("PREFIX_ACC", ".field()"));
            }

            refreshes.push_back(common::processTemplate(Templ, repl));
        };

    common::ReplacementMap repl;
    repl.insert(std::make_pair("NAME", common::nameToAccessCopy(name())));
    if (isVersionOptional()) {
        repl.insert(std::make_pair("LIST_ACC", ".field()"));
    }

    auto& countPrefix = obj.detachedCountPrefixFieldName();
    if (!countPrefix.empty()) {
        static const std::string Templ = 
            "auto realValue = field_#^#NAME#$#()#^#LIST_ACC#$#.value().size();";
        repl["REAL_VALUE"] = common::processTemplate(Templ, repl);
        processPrefixFunc(countPrefix, repl);
    }

    auto& lengthPrefix = obj.detachedLengthPrefixFieldName();
    if (!lengthPrefix.empty()) {
        static const std::string Templ = 
            "auto realValue = field_#^#NAME#$#()#^#LIST_ACC#$#.length();";
        repl["REAL_VALUE"] = common::processTemplate(Templ, repl);
        processPrefixFunc(lengthPrefix, repl);
    }

    auto& elemLengthPrefix = obj.detachedElemLengthPrefixFieldName();
    if (!elemLengthPrefix.empty()) {
        static const std::string Templ = 
            "std::size_t realValue =\n"
            "    field_#^#NAME#$#()#^#LIST_ACC#$#.value().empty() ?\n"
            "        0U : field_#^#NAME#$#()#^#LIST_ACC#$#.value()[0].length();";
        repl["REAL_VALUE"] = common::processTemplate(Templ, repl);
        processPrefixFunc(elemLengthPrefix, repl);
    }

    if (refreshes.empty()) {
        return common::emptyString();
    }

    static const std::string Templ = 
        "bool updated = false;\n"
        "#^#UPDATES#$#\n"
        "return updated;\n";

    common::ReplacementMap finalRepl;
    finalRepl.insert(std::make_pair("UPDATES", common::listToString(refreshes, "\n", common::emptyString())));
    return common::processTemplate(Templ, finalRepl);
}

bool ListField::hasCustomReadRefreshImpl() const
{
    return 
        (!listFieldDslObj().detachedCountPrefixFieldName().empty()) ||
        (!listFieldDslObj().detachedLengthPrefixFieldName().empty()) ||
        (!listFieldDslObj().detachedElemLengthPrefixFieldName().empty());
}

std::string ListField::getReadPreparationImpl(const FieldsList& fields) const
{
    auto obj = listFieldDslObj();
    bool versionOptional = isVersionOptional();
    common::StringsList preps;
    auto processPrefixFunc = 
        [&fields, &preps, versionOptional](const std::string& prefixName, const common::ReplacementMap& replacements)
        {
            if (prefixName.empty()) {
                return;
            }

            auto iter =
                std::find_if(
                    fields.begin(), fields.end(),
                    [&prefixName](auto& f)
                    {
                        return f->name() == prefixName;
                    });

            if (iter == fields.end()) {
                assert(!"Should not happen");
                return;
            }

            bool prefixVersionOptional = (*iter)->isVersionOptional();

            auto repl = replacements;
            repl.insert(std::make_pair("PREFIX_NAME", common::nameToAccessCopy(prefixName)));

            if ((!versionOptional) && (!prefixVersionOptional)) {
                static const std::string Templ =
                    "field_#^#NAME#$#().#^#FUNC#$#(\n"
                    "    static_cast<std::size_t>(field_#^#PREFIX_NAME#$#().value()));\n";

                preps.push_back(common::processTemplate(Templ, repl));
                return;
            }

            if ((versionOptional) && (!prefixVersionOptional)) {
                static const std::string Templ =
                    "if (field_#^#NAME#$#().doesExist()) {\n"
                    "    field_#^#NAME#$#().field().#^#FUNC#$#(\n"
                    "        static_cast<std::size_t>(field_#^#PREFIX_NAME#$#().value()));\n"
                    "}\n";

                preps.push_back(common::processTemplate(Templ, repl));
                return;
            }

            if ((!versionOptional) && (prefixVersionOptional)) {
                static const std::string Templ =
                    "if (field_#^#PREFIX_NAME#$#().doesExist()) {\n"
                    "    field_#^#NAME#$#().#^#FUNC#$#(\n"
                    "        static_cast<std::size_t>(field_#^#PREFIX_NAME#$#().field().value()));\n"
                    "}\n";

                preps.push_back(common::processTemplate(Templ, repl));
                return;
            }

            assert(versionOptional && prefixVersionOptional);
            static const std::string Templ =
                "if (field_#^#NAME#$#().doesExist() && field_#^#PREFIX_NAME#$#().doesExist()) {\n"
                "    field_#^#NAME#$#().field().#^#FUNC#$#(\n"
                "        static_cast<std::size_t>(field_#^#PREFIX_NAME#$#().field().value()));\n"
                "}\n";

            preps.push_back(common::processTemplate(Templ, repl));
            return;
        };

    common::ReplacementMap repl;
    repl.insert(std::make_pair("NAME", common::nameToAccessCopy(name())));
    
    auto& countPrefix = obj.detachedCountPrefixFieldName();
    if (!countPrefix.empty()) {
        repl["FUNC"] = "forceReadElemCount";
        processPrefixFunc(countPrefix, repl);
    }

    auto& lengthPrefix = obj.detachedLengthPrefixFieldName();
    if (!lengthPrefix.empty()) {
        repl["FUNC"] = "forceReadLength";
        processPrefixFunc(lengthPrefix, repl);
    }

    auto& elemLengthPrefix = obj.detachedElemLengthPrefixFieldName();
    if (!elemLengthPrefix.empty()) {
        repl["FUNC"] = "forceReadElemLength";
        processPrefixFunc(elemLengthPrefix, repl);
    }

    if (preps.empty()) {
        return common::emptyString();
    }

    return common::listToString(preps, "\n", common::emptyString());
}

bool ListField::isLimitedCustomizableImpl() const
{
    return true;
}

bool ListField::isVersionDependentImpl() const
{
    if (m_element && m_element->isVersionDependent()) {
        return true;
    }

    if (m_countPrefix && m_countPrefix->isVersionDependent()) {
        return true;
    }

    if (m_lengthPrefix && m_lengthPrefix->isVersionDependent()) {
        return true;
    }

    if (m_elemLengthPrefix && m_elemLengthPrefix->isVersionDependent()) {
        return true;
    }

    return false;
}
std::string ListField::getCommonDefinitionImpl(const std::string& fullScope) const
{
    common::StringsList defs;

    auto updatedScope = fullScope + common::membersSuffixStr() + "::";
    auto updateDefsFor =
        [&defs, updatedScope](const FieldPtr& f)
        {
            if (!f) {
                return;
            }

            auto str = f->getCommonDefinition(updatedScope);
            if (str.empty()) {
                return;
            }

            defs.emplace_back(std::move(str));
        };

    updateDefsFor(m_element);
    updateDefsFor(m_countPrefix);
    updateDefsFor(m_lengthPrefix);
    updateDefsFor(m_elemLengthPrefix);

    std::string membersCommon;
    if (!defs.empty()) {
        static const std::string Templ =
            "/// @brief Scope for all the common definitions of the member fields of\n"
            "///     @ref #^#SCOPE#$# list.\n"
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

std::string ListField::getExtraRefToCommonDefinitionImpl(const std::string& fullScope) const
{
    auto checkFunc =
        [](const FieldPtr& f) -> bool
        {
            return static_cast<bool>(f);
        };

    bool hasCommonMembers =
        checkFunc(m_element) ||
        checkFunc(m_countPrefix) ||
        checkFunc(m_lengthPrefix) ||
        checkFunc(m_elemLengthPrefix);

    if (!hasCommonMembers) {
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

std::string ListField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);
    checkFixedSizeOpt(options);
    checkCountPrefixOpt(options);
    checkLengthPrefixOpt(options);
    checkElemLengthPrefixOpt(options);
    checkDetachedPrefixOpt(options);

    return common::listToString(options, ",\n", common::emptyString());
}

std::string ListField::getElement() const
{
    if (m_element) {
        auto str =
            "typename " + common::nameToClassCopy(name()) +
            common::membersSuffixStr();

        if (!externalRef().empty()) {
            str += "<TOpt>";
        }

        str += "::";
        str += common::nameToClassCopy(m_element->name());
        return str;
    }

    auto obj = listFieldDslObj();
    auto elementField = obj.elementField();
    assert(elementField.valid());
    auto extRef = elementField.externalRef();
    assert(!extRef.empty());
    return generator().scopeForField(extRef, true, true) + "<TOpt>";
}

std::string ListField::getMembersDef(const std::string& scope) const
{
    auto membersScope =
        scope + common::nameToClassCopy(name()) +
        common::membersSuffixStr() + "::";

    StringsList defs;
    auto recordFieldFunc =
        [this](commsdsl::Field f)
        {
            assert(f.valid());
            auto extRef = f.externalRef();
            assert(!extRef.empty());
            auto* fieldPtr = generator().findField(extRef, true);
            static_cast<void>(fieldPtr);
            assert(fieldPtr != nullptr);
        };


    auto obj = listFieldDslObj();
    do {
        if (m_element) {
            defs.push_back(m_element->getClassDefinition(membersScope));
            break;
        }

        recordFieldFunc(obj.elementField());
    } while (false);

    do {
        if (m_countPrefix) {
            defs.push_back(m_countPrefix->getClassDefinition(membersScope));
            break;
        }

        if (!obj.hasCountPrefixField()) {
            break;
        }

        recordFieldFunc(obj.countPrefixField());
    } while (false);

    do {
        if (m_lengthPrefix) {
            defs.push_back(m_lengthPrefix->getClassDefinition(membersScope));
            break;
        }

        if (!obj.hasLengthPrefixField()) {
            break;
        }

        recordFieldFunc(obj.lengthPrefixField());
    } while (false);

    do {
        if (m_elemLengthPrefix) {
            defs.push_back(m_elemLengthPrefix->getClassDefinition(membersScope));
            break;
        }

        if (!obj.hasElemLengthPrefixField()) {
            break;
        }

        recordFieldFunc(obj.elemLengthPrefixField());
    } while (false);


    if (defs.empty()) {
        return common::emptyString();
    }

    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().scopeForOptions(common::defaultOptionsStr(), true, true) + ">";
    }

    static const std::string Templ =
        "/// @brief Scope for all the member fields of\n"
        "///     @ref #^#CLASS_NAME#$# list.\n"
        "#^#EXTRA_PREFIX#$#\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#MEMBERS_DEF#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("MEMBERS_DEF", common::listToString(defs, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

void ListField::checkFixedSizeOpt(ListField::StringsList& list) const
{
    auto obj = listFieldDslObj();
    auto fixedCount = obj.fixedCount();
    if (fixedCount == 0U) {
        return;
    }

    auto str =
        "comms::option::def::SequenceFixedSize<" +
        common::numToString(static_cast<std::uintmax_t>(fixedCount)) +
        ">";
    list.push_back(std::move(str));
}

void ListField::checkCountPrefixOpt(ListField::StringsList& list) const
{
    auto obj = listFieldDslObj();
    if (!obj.hasCountPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_countPrefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_countPrefix->name());
    }
    else {
        auto prefixField = obj.countPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    list.push_back("comms::option::def::SequenceSizeFieldPrefix<" + prefixName + '>');
}

void ListField::checkLengthPrefixOpt(ListField::StringsList& list) const
{
    auto obj = listFieldDslObj();
    if (!obj.hasLengthPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_lengthPrefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_lengthPrefix->name());
    }
    else {
        auto prefixField = obj.lengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    list.push_back("comms::option::def::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void ListField::checkElemLengthPrefixOpt(StringsList& list) const
{
    auto obj = listFieldDslObj();
    if (!obj.hasElemLengthPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_elemLengthPrefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_elemLengthPrefix->name());
    }
    else {
        auto prefixField = obj.elemLengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    std::string opt = "SequenceElemSerLengthFieldPrefix";
    if (obj.elemFixedLength()) {
        opt = "SequenceElemFixedSerLengthFieldPrefix";
    }

    list.push_back("comms::option::def::" + opt + "<" + prefixName + '>');
}

bool ListField::checkDetachedPrefixOpt(StringsList& list) const
{
    auto obj = listFieldDslObj();
    if (!obj.detachedCountPrefixFieldName().empty()) {
        list.push_back("comms::option::def::SequenceSizeForcingEnabled");
    }

    if (!obj.detachedLengthPrefixFieldName().empty()) {
        list.push_back("comms::option::def::SequenceLengthForcingEnabled");
    }

    if (!obj.detachedElemLengthPrefixFieldName().empty()) {
        list.push_back("comms::option::def::SequenceElemLengthForcingEnabled");
    }
    return true;
}

bool ListField::isElemForcedSerialisedHiddenInPlugin() const
{
    auto obj = listFieldDslObj();
    return obj.hasElemLengthPrefixField();
}

std::string ListField::getPrefixName() const
{
    auto obj = listFieldDslObj();
    do {
        if (!obj.hasCountPrefixField()) {
            break;
        }

        if (m_countPrefix) {
            return m_countPrefix->displayName();
        }

        auto countField = obj.countPrefixField();
        assert(countField.valid());
        return common::displayName(countField.displayName(), countField.name());
    } while (false);

    assert(obj.hasLengthPrefixField());

    if (m_lengthPrefix) {
        return m_lengthPrefix->displayName();
    }

    auto lengthPrefix = obj.lengthPrefixField();
    assert(lengthPrefix.valid());
    return common::displayName(lengthPrefix.displayName(), lengthPrefix.name());
}

std::string ListField::getExtraOptions(const std::string& scope, GetExtraOptionsFunc func, const std::string& base) const
{
    std::string nextBase;
    std::string ext;
    if (!base.empty()) {
        nextBase = base + "::" + common::nameToClassCopy(name()) + common::membersSuffixStr();
        ext = " : public " + nextBase;
    }   

    std::string memberScope =
        scope + common::nameToClassCopy(name()) +
        common::membersSuffixStr() + "::";
    StringsList options;

    auto addToOptionsFunc =
        [&nextBase, &memberScope, &options, func](const FieldPtr& f)
        {
            if (!f) {
                return;
            }

            auto o = (f.get()->*func)(nextBase, memberScope);
            if (!o.empty()) {
                options.push_back(o);
            }
        };

    addToOptionsFunc(m_element);
    addToOptionsFunc(m_lengthPrefix);
    addToOptionsFunc(m_countPrefix);
    addToOptionsFunc(m_elemLengthPrefix);

    if (options.empty()) {
        return common::emptyString();
    }

    const std::string Templ =
        "/// @brief Extra options for all the member fields of\n"
        "///     @ref #^#SCOPE#$##^#CLASS_NAME#$# list.\n"
        "struct #^#CLASS_NAME#$#Members#^#EXT#$#\n"
        "{\n"
        "    #^#OPTIONS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("EXT", std::move(ext)));
    replacements.insert(std::make_pair("OPTIONS", common::listToString(options, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}


} // namespace commsdsl2comms
