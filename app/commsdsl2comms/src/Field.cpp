//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "Field.h"

#include <functional>
#include <type_traits>
#include <cassert>
#include <algorithm>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "IntField.h"
#include "RefField.h"
#include "EnumField.h"
#include "SetField.h"
#include "FloatField.h"
#include "BitfieldField.h"
#include "OptionalField.h"
#include "BundleField.h"
#include "StringField.h"
#include "DataField.h"
#include "ListField.h"
#include "VariantField.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

const std::string& Field::displayName() const
{
    return common::displayName(m_dslObj.displayName(), m_dslObj.name());
}

std::size_t Field::minLength() const
{
    if (isVersionOptional()) {
        return 0U;
    }

    auto result = minLengthImpl();
    return result;
}

void Field::updateIncludes(Field::IncludesList& includes) const
{
    static const IncludesList CommonIncludes =  {
        "comms/options.h"
    };

    common::mergeIncludes(CommonIncludes, includes);
    common::mergeInclude(m_generator.headerfileForField(common::fieldBaseStr(), false), includes);

    if (!m_externalRef.empty()) {
        auto inc = m_generator.headerfileForOptions(common::defaultOptionsStr(), false);
        common::mergeInclude(inc, includes);
    }

    if (isVersionOptional()) {
        common::mergeInclude("comms/field/Optional.h", includes);
    }

    updateIncludesImpl(includes);
}

void Field::updatePluginIncludes(Field::IncludesList& includes) const
{
    if (!m_externalRef.empty()) {
        common::mergeInclude(m_generator.headerfileForFieldInPlugin(m_externalRef, false), includes);
    }
    updatePluginIncludesImpl(includes);
}

bool Field::doesExist() const
{
    return
        m_generator.doesElementExist(
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
            m_dslObj.isDeprecatedRemoved());
}

bool Field::prepare(unsigned parentVersion)
{
    m_externalRef = m_dslObj.externalRef();
    m_parentVersion = parentVersion;
    m_customRead = m_generator.getCustomReadForField(m_externalRef);
    m_customRefresh = m_generator.getCustomRefreshForField(m_externalRef);
    return prepareImpl();
}

std::string Field::getClassDefinition(
    const std::string& scope,
    const std::string& className) const
{
    std::string str;
    bool optional = isVersionOptional();

    auto classNameCpy(className);
    if (classNameCpy.empty()) {
        classNameCpy = common::nameToClassCopy(name());
    }

    if (optional) {
         classNameCpy += common::optFieldSuffixStr();
    }

    str += getClassDefinitionImpl(scope, classNameCpy);

    if (optional) {
        str += '\n';
        str += getClassPrefix(classNameCpy, false);

        static const std::string Templ =
            "struct #^#CLASS_NAME#$# : public\n"
            "    comms::field::Optional<\n"
            "        #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#,\n"
            "        comms::option::def::#^#DEFAULT_MODE_OPT#$#,\n"
            "        comms::option::def::#^#VERSIONS_OPT#$#\n"
            "    >\n"
            "{\n"
            "    /// @brief Name of the field.\n"
            "    static const char* name()\n"
            "    {\n"
            "        return #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#::name();\n"
            "    }\n"
            "};\n";

        std::string fieldParams;
        if (!m_externalRef.empty()) {
            fieldParams = "<TOpt, TExtraOpts...>";
        }

        std::string defaultModeOpt = "ExistsByDefault";
        if (!doesExist()) {
            defaultModeOpt = "MissingByDefault";
        }

        std::string versionOpt = "ExistsSinceVersion<" + common::numToString(m_dslObj.sinceVersion()) + '>';
        if (m_dslObj.isDeprecatedRemoved()) {
            assert(m_dslObj.deprecatedSince() < commsdsl::Protocol::notYetDeprecated());
            if (m_dslObj.sinceVersion() == 0U) {
                versionOpt = "ExistsUntilVersion<" + common::numToString(m_dslObj.deprecatedSince() - 1) + '>';
            }
            else {
                versionOpt =
                    "ExistsBetweenVersions<" +
                    common::numToString(m_dslObj.sinceVersion()) +
                    ", " +
                    common::numToString(m_dslObj.deprecatedSince() - 1) +
                    '>';
            }
        }

        classNameCpy = className;
        if (classNameCpy.empty()) {
            classNameCpy = common::nameToClassCopy(name());
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("CLASS_NAME", classNameCpy));
        replacements.insert(std::make_pair("FIELD_PARAMS", std::move(fieldParams)));
        replacements.insert(std::make_pair("DEFAULT_MODE_OPT", std::move(defaultModeOpt)));
        replacements.insert(std::make_pair("VERSIONS_OPT", std::move(versionOpt)));
        str += common::processTemplate(Templ, replacements);
    }
    return str;
}

std::string Field::getCommonPreDefinition(const std::string& scope) const
{
    if (isCommonPreDefDisabled()) {
        return common::emptyString();
    }

    return getCommonPreDefinitionImpl(scope);
}

Field::Ptr Field::create(Generator& generator, commsdsl::Field field)
{
    using CreateFunc = std::function<Ptr (Generator& generator, commsdsl::Field)>;
    static const CreateFunc Map[] = {
        /* Int */ [](Generator& g, commsdsl::Field f) { return createIntField(g, f); },
        /* Enum */ [](Generator& g, commsdsl::Field f) { return createEnumField(g, f); },
        /* Set */ [](Generator& g, commsdsl::Field f) { return createSetField(g, f); },
        /* Float */ [](Generator& g, commsdsl::Field f) { return createFloatField(g, f); },
        /* Bitfield */ [](Generator& g, commsdsl::Field f) { return createBitfieldField(g, f); },
        /* Bundle */ [](Generator& g, commsdsl::Field f) { return createBundleField(g, f); },
        /* String */ [](Generator& g, commsdsl::Field f) { return createStringField(g, f); },
        /* Data */ [](Generator& g, commsdsl::Field f) { return createDataField(g, f); },
        /* List */ [](Generator& g, commsdsl::Field f) { return createListField(g, f); },
        /* Ref */ [](Generator& g, commsdsl::Field f) { return createRefField(g, f); },
        /* Optional */ [](Generator& g, commsdsl::Field f) { return createOptionalField(g, f); },
        /* Variant */ [](Generator& g, commsdsl::Field f) { return createVariantField(g, f); },
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == (unsigned)commsdsl::Field::Kind::NumOfValues, "Invalid map");

    auto idx = static_cast<std::size_t>(field.kind());
    if (MapSize <= idx) {
        assert(!"Unexpected field kind");
        return Ptr();
    }

    return Map[idx](generator, field);
}

std::string Field::getDefaultOptions(const std::string& scope) const
{
    auto fullScope = scope;
    if (!m_externalRef.empty()) {
        fullScope += common::fieldStr() + "::";
    }

    auto str = getExtraDefaultOptionsImpl(fullScope);
    
    if (!isCustomizable()) {
        return str;
    }    

    if (!str.empty()) {
        str += '\n';
    }

    auto docStr = "/// @brief Extra options for @ref " +
        fullScope + common::nameToClassCopy(name()) + " field.";

    return
        str +
        common::makeDoxygenMultilineCopy(docStr, 40) +
        "\nusing " + common::nameToClassCopy(name()) +
            " = " + common::emptyOptionString() + ";\n";
}

std::string Field::getBareMetalDefaultOptions(const std::string& scope) const
{
    auto fullScope = scope;
    if (!m_externalRef.empty()) {
        fullScope += common::fieldStr() + "::";
    }

    auto str = getExtraBareMetalDefaultOptionsImpl(fullScope);
    
    if (!isCustomizable()) {
        return str;
    }   

    auto optStr = getBareMetalOptionStrImpl();
    if (optStr.empty()) {
        return str;
    } 

    if (!str.empty()) {
        str += '\n';
    }

    auto docStr = "/// @brief Extra options for @ref " +
        fullScope + common::nameToClassCopy(name()) + " field.";

    return
        str +
        common::makeDoxygenMultilineCopy(docStr, 40) +
        "\nusing " + common::nameToClassCopy(name()) +
            " = " + optStr + ";\n";
}

bool Field::writeFiles() const
{
    return
        writeProtocolDefinitionFile() &&
        writePluginHeaderFile() &&
        writePluginScrFile();
}

std::string Field::getClassPrefix(
    const std::string& className,
    bool checkForOptional,
    const std::string& extraDetails,
    const std::string& extraDoxygen) const
{
    std::string str;
    bool optional = checkForOptional && isVersionOptional();
    auto& doxygenPrefix = common::doxygenPrefixStr();

    if (optional) {
        std::string classNameCpy(className);
        if (common::optFieldSuffixStr().size() <= classNameCpy.size()) {
            classNameCpy.resize(classNameCpy.size() - common::optFieldSuffixStr().size());
        }

        str = "/// @brief Inner field of @ref " + classNameCpy + " optional.\n";
    }
    else {
        str = "/// @brief Definition of <b>\"";
        str += displayName();
        str += "\"</b> field.\n";

        auto& desc = m_dslObj.description();
        do {
            if (desc.empty() && extraDetails.empty()) {
                break;
            }

            str += "/// @details\n";

            if (!desc.empty()) {
                auto multiDesc = common::makeMultilineCopy(desc);
                common::insertIndent(multiDesc);
                multiDesc.insert(multiDesc.begin(), doxygenPrefix.begin(), doxygenPrefix.end());
                ba::replace_all(multiDesc, "\n", "\n" + doxygenPrefix);
                str += multiDesc;
                str += '\n';
            }

            if (extraDetails.empty()) {
                break;
            }

            if (!desc.empty()) {
                str += doxygenPrefix;
                str += '\n';
            }

            auto updateExtraDoc = common::insertIndentCopy(extraDetails);
            updateExtraDoc.insert(updateExtraDoc.begin(), doxygenPrefix.begin(), doxygenPrefix.end());
            ba::replace_all(updateExtraDoc, "\n", "\n" + doxygenPrefix);
            str += updateExtraDoc;
            str += '\n';
        } while (false);
    }

    if (!extraDoxygen.empty()) {
        auto updateExtraDoxygen = doxygenPrefix + extraDoxygen;
        ba::replace_all(updateExtraDoxygen, "\n", "\n" + doxygenPrefix);
        str += updateExtraDoxygen;
        str += '\n';
    }

    if (!m_externalRef.empty()) {
        str += "/// @tparam TOpt Protocol options.\n";
        str += "/// @tparam TExtraOpts Extra options.\n";
        str += "template <typename TOpt = ";
        str += m_generator.scopeForOptions(common::defaultOptionsStr(), true, true);
        str += ", typename... TExtraOpts>\n";
    }

    return str;
}

std::string Field::dslCondToString(
    const FieldsList& fields,
    const commsdsl::OptCond& cond,
    bool bracketsWrap)
{
    if (cond.kind() == commsdsl::OptCond::Kind::Expr) {
        auto findFieldFunc =
            [&fields](const std::string& name) -> const Field*
            {
                auto iter =
                    std::find_if(
                        fields.begin(), fields.end(),
                        [&name](auto& f)
                        {
                            return f->name() == name;
                        });

                if (iter == fields.end()) {
                    return nullptr;
                }

                return iter->get();
            };

        auto opFunc =
            [](const std::string& val) -> const std::string& {
                if (val == "=") {
                    static const std::string Str = "==";
                    return Str;
                }
                return val;
            };

        commsdsl::OptCondExpr exprCond(cond);
        auto& left = exprCond.left();
        auto& op = opFunc(exprCond.op());
        auto& right = exprCond.right();

        if (!left.empty()) {
            assert(!op.empty());
            assert(!right.empty());
            assert(left[0] == '$');

            std::string leftFieldName(left, 1);
            auto* leftField = findFieldFunc(leftFieldName);
            if (leftField == nullptr) {
                assert(!"Should not happen");
                return common::emptyString();
            }

            if (right[0] != '$') {
                return leftField->getCompareToValue(op, right);
            }

            auto* rightField = findFieldFunc(std::string(right, 1));
            if (rightField == nullptr) {
                assert(!"Should not happen");
                return common::emptyString();
            }

            return leftField->getCompareToField(op, *rightField);
        }

        // Reference to bit in "set".
        if (right[0] != '$') {
            assert(!"Should not happen");
            return common::emptyString();
        }

        std::string fieldRef(right, 1U);
        auto dotPos = fieldRef.find(".");
        std::string fieldExternalRef(fieldRef, 0, dotPos);
        auto* rightField = findFieldFunc(fieldExternalRef);

        if (rightField == nullptr) {
            assert(!"Should not happen");
            return common::emptyString();
        }

        assert(
            (rightField->kind() == commsdsl::Field::Kind::Set) ||
            (rightField->kind() == commsdsl::Field::Kind::Ref));
        std::string valueStr;
        if (dotPos != std::string::npos) {
            valueStr.assign(fieldRef.begin() + dotPos + 1, fieldRef.end());
        }

        return rightField->getCompareToValue(op, valueStr);
    }

    if ((cond.kind() != commsdsl::OptCond::Kind::List)) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    commsdsl::OptCondList listCond(cond);
    auto type = listCond.type();

    static const std::string AndOp = " &&\n";
    static const std::string OrOp = " ||\n";

    auto* op = &AndOp;
    if (type == commsdsl::OptCondList::Type::Or) {
        op = &OrOp;
    }
    else {
        assert(type == commsdsl::OptCondList::Type::And);
    }

    auto conditions = listCond.conditions();
    std::string condTempl;
    common::ReplacementMap replacements;
    if (bracketsWrap) {
        condTempl += '(';
    }

    for (auto count = 0U; count < conditions.size(); ++count) {
        if (0U < count) {
            condTempl += ' ';
        }

        auto condStr = "COND" + std::to_string(count);
        replacements.insert(std::make_pair(condStr, dslCondToString(fields, conditions[count], true)));
        condTempl += "(#^#";
        condTempl += condStr;
        condTempl += "#$#)";
        if (count < (conditions.size() - 1U)) {
            condTempl += *op;
        }
    }

    if (bracketsWrap) {
        condTempl += ')';
    }

    return common::processTemplate(condTempl, replacements);
}

bool Field::isVersionOptional() const
{
    if (!m_generator.versionDependentCode()) {
        return false;
    }

    if (!m_generator.isElementOptional(m_dslObj.sinceVersion(), m_dslObj.deprecatedSince(), m_dslObj.isDeprecatedRemoved())) {
        return false;
    }

    if (m_parentVersion < m_dslObj.sinceVersion()) {
        return true;
    }
    
    if ((m_dslObj.deprecatedSince() < commsdsl::Protocol::notYetDeprecated()) &&
        (m_dslObj.isDeprecatedRemoved())) {
        return true;
    }

    return false;
}

bool Field::isVersionDependent() const
{
    if (!m_generator.versionDependentCode()) {
        return false;
    }

    return isVersionOptional() || isVersionDependentImpl();
}

bool Field::isPseudo() const
{
    return m_forcedPseudo || m_dslObj.isPseudo();
}

std::string Field::getReadForFields(
    const FieldsList& fields,
    bool forMessage,
    bool updateVersion)
{
    std::vector<std::size_t> customReadFields;
    for (std::size_t idx = 0U; idx < fields.size(); ++idx) {
        auto& m = fields[idx];

        assert(m);
        if (m->hasCustomReadRefresh()) {
            customReadFields.push_back(idx);
        }
    }

    if (customReadFields.empty()) {
        return common::emptyString();
    }

    std::string readFromStr("readFrom");
    std::string readUntilStr("readUntilAndUpdateLen");
    std::string readFromUntilStr("readFromUntilAndUpdateLen");
    std::string readFunc("read");
    if (forMessage) {
        readFromStr = "doReadFrom";
        readUntilStr = "doReadUntilAndUpdateLen";
        readFromUntilStr = "doReadFromUntilAndUpdateLen";
        readFunc = "doRead";
    }

    bool esDeclared = false;
    auto esAssignment =
        [&esDeclared]() -> std::string
        {
            std::string str;
            if (!esDeclared) {
                str += "auto ";
                esDeclared = true;
            }
            str += "es = ";
            return str;
        };

    common::StringsList reads;
    for (auto fPosIdx = 0U; fPosIdx < customReadFields.size(); ++fPosIdx) {
        auto fPos = customReadFields[fPosIdx];
        assert(fPos != 0);
        auto& m = fields[fPos];
        auto accessName = common::nameToAccessCopy(m->name());
        if ((!esDeclared) && (fPos != 0U)) {
            auto str = 
                esAssignment() + "Base::template " + readUntilStr + "<FieldIdx_" + accessName + ">(iter, len);\n"
                "if (es != comms::ErrorStatus::Success) {\n"
                "    return es;\n"
                "}\n";

            reads.push_back(std::move(str));
        }

        auto readPreparation = m->getReadPreparation(fields);
        assert(!readPreparation.empty());
        reads.push_back(std::move(readPreparation));

        if (fPos == customReadFields.back()) {
            auto str = 
                esAssignment() + "Base::template " + readFromStr + "<FieldIdx_" + accessName + ">(iter, len);\n"
                "if (es != comms::ErrorStatus::Success) {\n"
                "    return es;\n"
                "}\n";
            reads.push_back(std::move(str));
            continue;
        }

        assert((fPosIdx + 1) < customReadFields.size());
        auto nextName = common::nameToAccessCopy(fields[customReadFields[fPosIdx + 1]]->name());
        auto str = 
            esAssignment() + "Base::template " + readFromUntilStr + "<FieldIdx_" + accessName + ", FieldIdx_" + nextName + ">(iter, len);\n"
            "if (es != comms::ErrorStatus::Success) {\n"
            "    return es;\n"
            "}\n";
        reads.push_back(std::move(str));
    }

    static const std::string Templ =
        "/// @brief Custom read functionality.\n"
        "template <typename TIter>\n"
        "comms::ErrorStatus #^#READ_FUNC#$#(TIter& iter, std::size_t len)\n"
        "{\n"
        "    #^#UPDATE_VERSION#$#\n"
        "    #^#READS#$#\n"
        "    return comms::ErrorStatus::Success;\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("READS", common::listToString(reads, "\n", common::emptyString())));
    replacements.insert(std::make_pair("READ_FUNC", std::move(readFunc)));

    if (updateVersion) {
        assert(forMessage);
        replacements.insert(std::make_pair("UPDATE_VERSION", "Base::doFieldsVersionUpdate();\n"));
    }
    return common::processTemplate(Templ, replacements);
}

std::string Field::getPublicRefreshForFields(
    const Field::FieldsList& fields, 
    bool forMessage)
{
    common::StringsList calls;
    for (auto& m : fields) {
        assert(m);
        if (!m->hasCustomReadRefresh()) {
            continue;
        }

        auto str =
            "updated = refresh_" +
            common::nameToAccessCopy(m->name()) +
            "() || updated;";
        calls.push_back(std::move(str));
    }

    if (calls.empty()) {
        return common::emptyString();
    }

    std::string func("refresh");
    if (forMessage) {
        func = "doRefresh";
    }

    static const std::string Templ =
        "/// @brief Custom refresh functionality.\n"
        "bool #^#FUNC#$#()\n"
        "{\n"
        "    bool updated = Base::#^#FUNC#$#();\n"
        "    #^#CALLS#$#\n"
        "    return updated;\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CALLS", common::listToString(calls, "\n", common::emptyString())));
    replacements.insert(std::make_pair("FUNC", std::move(func)));

    return common::processTemplate(Templ, replacements);
}

std::string Field::getPrivateRefreshForFields(const Field::FieldsList& fields)
{
    common::StringsList funcs;
    for (auto& m : fields) {
        assert(m);
        auto body = m->getPrivateRefreshBody(fields);
        if (body.empty()) {
            continue;
        }

        static const std::string Templ =
            "bool refresh_#^#NAME#$#()\n"
            "{\n"
            "    #^#BODY#$#\n"
            "}\n";

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(m->name())));
        replacements.insert(std::make_pair("BODY", std::move(body)));
        funcs.push_back(common::processTemplate(Templ, replacements));
    }

    if (funcs.empty()) {
        return common::emptyString();
    }

    return common::listToString(funcs, "\n", common::emptyString());
}

std::string Field::getPluginCreatePropsFunc(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    static const std::string Templ = 
        "#^#ANON_NAMESPACE#$#\n"
        "static QVariantMap createProps_#^#NAME#$#(#^#SER_HIDDEN#$#)\n"
        "{\n"
        "    #^#SER_HIDDEN_CAST#$#\n"
        "    #^#BODY#$#\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(name())));
    replacements.insert(std::make_pair("BODY", getPluginPropsDefFuncBodyImpl(scope, false, forcedSerialisedHidden, serHiddenParam)));
    replacements.insert(std::make_pair("ANON_NAMESPACE", getPluginAnonNamespace(scope, forcedSerialisedHidden, serHiddenParam)));
    if (serHiddenParam) {
        replacements.insert(std::make_pair("SER_HIDDEN", "bool serHidden"));
    }

    return common::processTemplate(Templ, replacements);
}

std::string Field::getPluginAnonNamespace(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    auto scopeCpy = scope;
    if (scopeCpy.empty()) {
        scopeCpy = m_generator.scopeForField(m_externalRef, true, false);
    }
    auto str = getPluginAnonNamespaceImpl(scopeCpy, forcedSerialisedHidden, serHiddenParam);
    if (str.empty()) {
        return common::emptyString();
    }

    if (!scope.empty()) {
        return str;
    }

    static const std::string Templ =
        "namespace\n"
        "{\n\n"
        "#^#STR#$#\n"
        "} // namespace\n\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("STR", std::move(str)));
    return common::processTemplate(Templ, replacements);
}

bool Field::prepareImpl()
{
    return true;
}

void Field::updateIncludesImpl(IncludesList& includes) const
{
    static_cast<void>(includes);
}

void Field::updatePluginIncludesImpl(IncludesList& includes) const
{
    static_cast<void>(includes);
}

std::size_t Field::minLengthImpl() const
{
    return m_dslObj.minLength();
}

std::size_t Field::maxLengthImpl() const
{
    return m_dslObj.maxLength();
}

std::string Field::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    static_cast<void>(scope);
    return common::emptyString();
}

std::string Field::getExtraBareMetalDefaultOptionsImpl(const std::string& scope) const
{
    static_cast<void>(scope);
    return common::emptyString();
}

std::string Field::getBareMetalOptionStrImpl() const
{
    return "comms::option::app::EmptyOption";
}

std::string Field::getCompareToValueImpl(const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    bool versionOptional = forcedVersionOptional || isVersionOptional();
    if (versionOptional) {
        return
            "field_" + usedName + "().value() " +
            op + ' ' + value;
    }

    return
        "field_" + usedName + "().doesExist() &&\n" +
        "(field_" + usedName + "().field().value() " +
        op + ' ' + value + ')';

}

std::string Field::getCompareToFieldImpl(const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = common::nameToAccessCopy(name());
    }

    auto fieldName = common::nameToAccessCopy(field.name());
    bool thisOptional = forcedVersionOptional || isVersionOptional();
    bool otherOptional = field.isVersionOptional();

    std::string thisFieldValue;
    if (thisOptional) {
        thisFieldValue = "field_" + usedName + "().field().value() ";
    }
    else {
        thisFieldValue = "field_" + usedName + "().value() ";
    }

    std::string otherFieldValue;
    if (otherOptional) {
        otherFieldValue = " field_" + fieldName + "().field().value()";
    }
    else {
        otherFieldValue = " field_" + fieldName + "().value()";
    }

    auto compareExpr = thisFieldValue + op + otherFieldValue;

    if ((!thisOptional) && (!otherOptional)) {
        return compareExpr;
    }


    if ((!thisOptional) && (otherOptional)) {
        return "field_" + fieldName + "().doesExist() &&\n(" + compareExpr + ')';
    }

    if ((thisOptional) && (!otherOptional)) {
        return "field_" + usedName + "().doesExist() &&\n(" + compareExpr + ')';
    }

    return
        "field_" + usedName + "().doesExist() &&\n"
        "field_" + fieldName + "().doesExist() &&\n"
                               "(" + compareExpr + ')';
}

std::string Field::getPluginPropsDefFuncBodyImpl(
    const std::string& scope,
    bool externalName,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    static const std::string Templ =
        "#^#SER_HIDDEN_CAST#$#\n"
        "using Field = #^#FIELD_SCOPE#$##^#CLASS_NAME#$##^#TEMPL_PARAMS#$#;\n"
        "return\n"
        "    cc::property::field::ForField<Field>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        #^#SER_HIDDEN#$#\n"
        "        #^#READ_ONLY#$#\n"          
        "        #^#HIDDEN#$#\n"
        "        #^#PROPERTIES#$#\n"
        "        .asMap();\n";

    static const std::string VerOptTempl =
        "#^#SER_HIDDEN_CAST#$#\n"
        "using InnerField = #^#FIELD_SCOPE#$##^#CLASS_NAME#$#Field;\n"
        "auto props =\n"
        "    cc::property::field::ForField<InnerField>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        #^#SER_HIDDEN#$#\n"
        "        #^#READ_ONLY#$#\n"      
        "        #^#HIDDEN#$#\n"          
        "        #^#PROPERTIES#$#\n"
        "        .asMap();\n\n"
        "using Field = #^#FIELD_SCOPE#$##^#CLASS_NAME#$#;\n"
        "return\n"
        "    cc::property::field::ForField<Field>()\n"
        "        .name(#^#NAME_PROP#$#)\n"
        "        .uncheckable()\n"
        "        .field(std::move(props))\n"
        "        .asMap();\n";

    bool verOptional = isVersionOptional();
    auto* templ = &Templ;
    if (verOptional) {
        templ = &VerOptTempl;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    if (!scope.empty()) {
        replacements.insert(std::make_pair("FIELD_SCOPE", scope));
    }
    else {
        replacements.insert(std::make_pair("FIELD_SCOPE", m_generator.scopeForField(m_externalRef, true, false)));
    }
    replacements.insert(std::make_pair("PROPERTIES", getPluginPropertiesImpl(serHiddenParam)));

    if (externalName) {
        replacements.insert(std::make_pair("NAME_PROP", "name"));
        replacements.insert(std::make_pair("TEMPL_PARAMS", "<>"));
    }
    else if (verOptional) {
        replacements.insert(std::make_pair("NAME_PROP", "InnerField::name()"));
    }
    else {
        replacements.insert(std::make_pair("NAME_PROP", "Field::name()"));
    }

    if (serHiddenParam) {
        replacements.insert(std::make_pair("SER_HIDDEN_CAST", "static_cast<void>(serHidden);"));
    }

    if (forcedSerialisedHidden || m_forcedPseudo || m_dslObj.isPseudo()) {
        replacements.insert(std::make_pair("SER_HIDDEN", ".serialisedHidden()"));
    }
    else if (serHiddenParam) {
        replacements.insert(std::make_pair("SER_HIDDEN", ".serialisedHidden(serHidden)"));
    }

    if (m_dslObj.isDisplayReadOnly()) {
        replacements.insert(std::make_pair("READ_ONLY", ".readOnly()"));
    }

    if (m_dslObj.isDisplayHidden()) {
        replacements.insert(std::make_pair("HIDDEN", ".hidden()"));
    }

    return common::processTemplate(*templ, replacements);
}

std::string Field::getPluginAnonNamespaceImpl(
    const std::string& scope,
    bool forcedSerialisedHidden,
    bool serHiddenParam) const
{
    static_cast<void>(scope);
    static_cast<void>(forcedSerialisedHidden);
    static_cast<void>(serHiddenParam);
    return common::emptyString();
}

std::string Field::getPluginPropertiesImpl(bool serHiddenParam) const
{
    static_cast<void>(serHiddenParam);
    return common::emptyString();
}

std::string Field::getPrivateRefreshBodyImpl(const FieldsList& fields) const
{
    static_cast<void>(fields);
    return common::emptyString();
}

bool Field::hasCustomReadRefreshImpl() const
{
    return false;
}

std::string Field::getReadPreparationImpl(const FieldsList& fields) const
{
    static_cast<void>(fields);
    return common::emptyString();
}

bool Field::isLimitedCustomizableImpl() const
{
    return false;
}

void Field::setForcedPseudoImpl()
{
    // Do nothing
}

void Field::setForcedNoOptionsConfigImpl()
{
    // Do nothing
}

bool Field::isVersionDependentImpl() const
{
    return false;
}

std::string Field::getCommonPreDefinitionImpl(const std::string& scope) const
{
    static_cast<void>(scope);
    return common::emptyString();
}

std::string Field::getNameFunc() const
{
    auto customName = m_generator.getCustomNameForField(m_externalRef);
    if (!customName.empty()) {
        return customName;
    }

    return
        "/// @brief Name of the field.\n"
        "static const char* name()\n"
        "{\n"
        "    return \"" + displayName() + "\";\n"
        "}\n";
}

void Field::updateExtraOptions(
    const std::string& scope,
    common::StringsList& options,
    bool ignoreFailOnInvalid) const
{
    if (!m_externalRef.empty()) {
        options.push_back("TExtraOpts...");
    }

    if ((!scope.empty()) && (isCustomizable())) {
        options.push_back("typename " + scope + common::nameToClassCopy(name()));
    }

    if ((!ignoreFailOnInvalid) && m_focedFailOnInvalid) {
        options.push_back("comms::option::def::FailOnInvalid<comms::ErrorStatus::ProtocolError>");
    }
    else if ((!ignoreFailOnInvalid) && m_dslObj.isFailOnInvalid()) {
        common::addToList("comms::option::def::FailOnInvalid<>", options);
    }

    if (!m_customRead.empty()) {
        common::addToList("comms::option::def::HasCustomRead", options);
    }

    if (!m_customRefresh.empty()) {
        common::addToList("comms::option::def::HasCustomRefresh", options);
    }

    if (m_forcedPseudo || m_dslObj.isPseudo()) {
        common::addToList("comms::option::def::EmptySerialization", options);
    }


}

const std::string& Field::getCustomRead() const
{
    return m_customRead;
}

std::string Field::getCustomWrite() const
{
    return m_generator.getCustomWriteForField(m_externalRef);
}

std::string Field::getCustomLength() const
{
    return m_generator.getCustomLengthForField(m_externalRef);
}

std::string Field::getCustomValid() const
{
    return m_generator.getCustomValidForField(m_externalRef);
}

const std::string& Field::getCustomRefresh() const
{
    return m_customRefresh;
}

std::string Field::getExtraPublic() const
{
    return m_generator.getExtraPublicForField(m_externalRef);
}

std::string Field::getExtraProtected() const
{
    return m_generator.getExtraProtectedForField(m_externalRef);
}

std::string Field::getFullProtected() const
{
    auto str = getExtraProtected();
    if (str.empty()) {
        return str;
    }

    common::insertIndent(str);
    return "protected:\n" + str;
}

std::string Field::getExtraPrivate() const
{
    return m_generator.getExtraPrivateForField(m_externalRef);
}

std::string Field::getFullPrivate() const
{
    auto str = getExtraPrivate();
    if (str.empty()) {
        return str;
    }

    common::insertIndent(str);
    return "private:\n" + str;
}

std::string Field::getCommonFieldBaseParams(commsdsl::Endian endian) const
{
    auto schemaEndian = generator().schemaEndian();
    assert(endian < commsdsl::Endian_NumOfValues);
    assert(schemaEndian < commsdsl::Endian_NumOfValues);

    if ((schemaEndian == endian) ||
        (commsdsl::Endian_NumOfValues <= endian)) {
        return common::emptyString();
    }

    return common::dslEndianToOpt(endian);
}

bool Field::isCustomizable() const
{
    if (m_generator.customizationLevel() == CustomizationLevel::Full) {
        return true;
    }

    if (m_dslObj.isCustomizable()) {
        return true;
    }

    if (m_generator.customizationLevel() == CustomizationLevel::None) {
        return false;
    }

    return isLimitedCustomizableImpl();
}

std::string Field::adjustScopeWithNamespace(const std::string& scope) const
{
    static const std::string OptPrefix("TOpt");
    if (ba::starts_with(scope, OptPrefix)) {
        return generator().mainNamespace() + scope.substr(OptPrefix.size());
    }
    return scope;
}

std::string Field::scopeForCommon(const std::string& scope) const
{
    static const std::string CommonStr("Common::");
    auto adjustedScope = ba::replace_all_copy(scope, "::", CommonStr);

    auto restorePrefixFunc =
        [this, &adjustedScope](const std::string& ns)
        {
            auto resultPrefix = generator().mainNamespace() + "::" + ns + "::";
            auto wrongPrefix = ba::replace_all_copy(resultPrefix, "::", CommonStr);
            if (ba::starts_with(adjustedScope, wrongPrefix)) {
                ba::replace_first(adjustedScope, wrongPrefix, resultPrefix);
            }
        };

    restorePrefixFunc(common::messageStr());
    restorePrefixFunc(common::fieldStr());
    restorePrefixFunc(common::frameStr());

    return adjustedScope;
}

bool Field::writeProtocolDefinitionFile() const
{
    auto startInfo = m_generator.startFieldProtocolWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        return true;
    }

    assert(!m_externalRef.empty());
    IncludesList includes;
    updateIncludes(includes);
    auto incStr = common::includesToStatements(includes);
    if (!m_externalRef.empty()) {
        incStr += m_generator.getExtraIncludeForField(m_externalRef);
    }

    auto namespaces = m_generator.namespacesForField(m_externalRef);

    auto scope = "TOpt::" + m_generator.scopeForField(m_externalRef);
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("INCLUDES", std::move(incStr)));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_DEF", getClassDefinition(scope, className)));
    replacements.insert(std::make_pair("CLASS_PRE_DEF", getCommonPreDefinition(scope)));
    replacements.insert(std::make_pair("FIELD_NAME", displayName()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForField(m_externalRef)));

    static const std::string FileTemplate(
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#FIELD_NAME#$#\"</b> field.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "#^#CLASS_PRE_DEF#$#\n"
        "#^#CLASS_DEF#$#\n"
        "#^#END_NAMESPACE#$#\n"
        "#^#APPEND#$#\n"
    );

    std::string str = common::processTemplate(FileTemplate, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Field::writePluginHeaderFile() const
{
    auto startInfo = m_generator.startFieldPluginHeaderWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        return true;
    }

    static const std::string Templ(
        "#pragma once\n"
        "\n"
        "#include <QtCore/QVariantMap>\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "QVariantMap createProps_#^#NAME#$#(const char* name, bool serHidden = false);\n\n"
        "#^#END_NAMESPACE#$#\n"
    );

    auto namespaces = m_generator.namespacesForFieldInPlugin(m_externalRef);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(className)));
    auto str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Field::writePluginScrFile() const
{
    assert(!isVersionOptional());
    auto startInfo = m_generator.startFieldPluginSrcWrite(m_externalRef);
    auto& filePath = startInfo.first;
    auto& className = startInfo.second;

    if (filePath.empty()) {
        return true;
    }

    static const std::string Templ(
        "#include \"#^#CLASS_NAME#$#.h\"\n"
        "\n"
        "#^#INCLUDES#$#\n\n"
        "namespace cc = comms_champion;\n\n"
        "#^#BEGIN_NAMESPACE#$#\n"
        "#^#ANON_NAMESPACE#$#\n"
        "QVariantMap createProps_#^#NAME#$#(const char* name, bool serHidden)\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n\n"
        "#^#END_NAMESPACE#$#\n"
    );

    auto namespaces = m_generator.namespacesForFieldInPlugin(m_externalRef);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(className)));
    replacements.insert(std::make_pair("ANON_NAMESPACE", getPluginAnonNamespace()));
    replacements.insert(std::make_pair("INCLUDES", getPluginIncludes()));
    replacements.insert(std::make_pair("BODY", getPluginPropsDefFuncBodyImpl(common::emptyString(), true, false, true)));

    auto str = common::processTemplate(Templ, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Field::getPluginIncludes() const
{
    IncludesList includes;
    common::mergeInclude("comms_champion/property/field.h", includes);
    common::mergeInclude(m_generator.headerfileForField(m_externalRef, false), includes);
    updatePluginIncludesImpl(includes);
    return common::includesToStatements(includes);
}

//std::string Field::getClassPreDefinitionInternal(const std::string& scope, const std::string& className) const
//{
//    auto str = getCommonPreDefinition(scope);
//    if (str.empty()) {
//        return str;
//    }

//    static const std::string Templ =
//        "/// @brief Common definitions for field @ref #^#SCOPE#$##^#CLASS_NAME#$#\n"
//        "struct #^#ORIG_CLASS_NAME#$#Common\n"
//        "{\n"
//        "    #^#DEFS#$#\n"
//        "};\n";

//    common::ReplacementMap repl;
//    repl.insert(std::make_pair("SCOPE", scope));
//    repl.insert(std::make_pair("CLASS_NAME", className));
//    repl.insert(std::make_pair("ORIG_CLASS_NAME", common::nameToClassCopy(name())));
//    repl.insert(std::make_pair("DEF", std::move(str)));
//    return common::processTemplate(Templ, repl);
//}

} // namespace commsdsl2comms
