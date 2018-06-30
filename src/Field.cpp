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
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string FileTemplate(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#FIELD_NAME#$#\"<\\b> field.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "#^#CLASS_DEF#$#\n"
    "#^#END_NAMESPACE#$#\n"
);

Field::IncludesList prepareCommonIncludes(const Generator& generator)
{
    Field::IncludesList list = {
        "comms/options.h",
        generator.mainNamespace() + '/' + common::fieldBaseStr() + common::headerSuffix(),
    };

    return list;
}

} // namespace

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
    static const IncludesList CommonIncludes = prepareCommonIncludes(m_generator);
    common::mergeIncludes(CommonIncludes, includes);
    if (!m_externalRef.empty()) {
        auto inc =
            m_generator.mainNamespace() + '/' + common::defaultOptionsStr() + common::headerSuffix();
        common::mergeInclude(inc, includes);
    }

    if (isVersionOptional()) {
        common::mergeInclude("comms/field/Optional.h", includes);
    }

    updateIncludesImpl(includes);
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
    return prepareImpl();
}

std::string Field::getClassDefinition(
    const std::string& scope,
    const std::string& suffix) const
{
    std::string str;
    bool optional = isVersionOptional();

    std::string classNameSuffix = suffix;
    if (optional) {
        classNameSuffix += common::optFieldSuffixStr();
    }

    str += getClassDefinitionImpl(scope, classNameSuffix);

    if (optional) {
        str += '\n';
        str += getClassPrefix(suffix, false);

        static const std::string Templ =
            "using #^#CLASS_NAME#$# =\n"
            "    comms::field::Optional<\n"
            "        #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#,\n"
            "        comms::option::#^#DEFAULT_MODE_OPT#$#,\n"
            "        comms::option::#^#VERSIONS_OPT#$#\n"
            "    >;\n";

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
                versionOpt = "ExistsUntilVersion<" + common::numToString(m_dslObj.deprecatedSince()) + '>';
            }
            else {
                versionOpt =
                    "ExistsBetweenVersions<" +
                    common::numToString(m_dslObj.sinceVersion()) +
                    ", " +
                    common::numToString(m_dslObj.deprecatedSince()) +
                    '>';
            }
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(m_dslObj.name()) + suffix));
        replacements.insert(std::make_pair("FIELD_PARAMS", std::move(fieldParams)));
        replacements.insert(std::make_pair("DEFAULT_MODE_OPT", std::move(defaultModeOpt)));
        replacements.insert(std::make_pair("VERSIONS_OPT", std::move(versionOpt)));
        str += common::processTemplate(Templ, replacements);
    }
    return str;
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
        /* Data */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* List */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* Ref */ [](Generator& g, commsdsl::Field f) { return createRefField(g, f); },
        /* Optional */ [](Generator& g, commsdsl::Field f) { return createOptionalField(g, f); },
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
    if (!str.empty()) {
        str += '\n';
    }

    return
        str +
        "/// @brief Extra options for @ref " +
        fullScope + common::nameToClassCopy(name()) + " field.\n" +
        "using " + common::nameToClassCopy(name()) +
        " = comms::option::EmptyOption;\n";
}

bool Field::writeProtocolDefinition() const
{
    auto startInfo = m_generator.startFieldProtocolWrite(m_externalRef);
    auto& filePath = startInfo.first;
    if (filePath.empty()) {
        return true;
    }

    assert(!m_externalRef.empty());
    IncludesList includes;
    updateIncludes(includes);
    auto incStr = common::includesToStatements(includes);

    auto namespaces = m_generator.namespacesForField(m_externalRef);

    // TODO: modify class name

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("INCLUDES", std::move(incStr)));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_DEF", getClassDefinition("TOpt::" + m_generator.scopeForField(m_externalRef))));
    replacements.insert(std::make_pair("FIELD_NAME", getDisplayName()));

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

const std::string& Field::getDisplayName() const
{
    auto* displayName = &m_dslObj.displayName();
    if (displayName->empty()) {
        displayName = &m_dslObj.name();
    }
    return *displayName;
}

std::string Field::getClassPrefix(
    const std::string& suffix,
    bool checkForOptional,
    const std::string& extraDoc) const
{
    std::string str;
    bool optional = checkForOptional && isVersionOptional();
    if (optional) {
        std::string suffixCpy(suffix);
        if (common::optFieldSuffixStr().size() <= suffixCpy.size()) {
            suffixCpy.resize(suffixCpy.size() - common::optFieldSuffixStr().size());
        }

        str = "/// @brief Inner field of @ref " + common::nameToClassCopy(name()) + suffixCpy + " optional.\n";
    }
    else {
        str = "/// @brief Definition of <b>\"";
        str += getDisplayName();
        str += "\"<\\b> field.\n";

        auto& desc = m_dslObj.description();
        do {
            if (desc.empty() && extraDoc.empty()) {
                break;
            }


            str += "/// @details\n";
            auto& doxygenPrefix = common::doxygenPrefixStr();

            if (!desc.empty()) {
                auto multiDesc = common::makeMultilineCopy(desc);
                common::insertIndent(multiDesc);
                multiDesc.insert(multiDesc.begin(), doxygenPrefix.begin(), doxygenPrefix.end());
                ba::replace_all(multiDesc, "\n", "\n" + doxygenPrefix);
                str += multiDesc;
                str += '\n';
            }

            if (extraDoc.empty()) {
                break;
            }

            if (!desc.empty()) {
                str += doxygenPrefix;
                str += '\n';
            }

            auto updateExtraDoc = common::insertIndentCopy(extraDoc);
            updateExtraDoc.insert(updateExtraDoc.begin(), doxygenPrefix.begin(), doxygenPrefix.end());
            ba::replace_all(updateExtraDoc, "\n", "\n" + doxygenPrefix);
            str += updateExtraDoc;
            str += '\n';
        } while (false);
    }

    if (!m_externalRef.empty()) {
        str += "/// @tparam TOpt Protocol options.\n";
        str += "/// @tparam TExtraOpts Extra options.\n";
        str += "template <typename TOpt = ";
        str += m_generator.mainNamespace();
        str += "::";
        str += common::defaultOptionsStr();
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

        assert(rightField->kind() == commsdsl::Field::Kind::Set);
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
    return
        (m_generator.versionDependentCode()) &&
        (m_parentVersion < m_dslObj.sinceVersion()) &&
        (m_generator.isElementOptional(
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
            m_dslObj.isDeprecatedRemoved()));
}

bool Field::prepareImpl()
{
    return true;
}

void Field::updateIncludesImpl(IncludesList& includes) const
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

std::string Field::getNameFunc() const
{
    return
        "/// @brief Name of the field.\n"
        "static const char* name()\n"
        "{\n"
        "    return \"" + getDisplayName() + "\";\n"
        "}\n";
}

void Field::updateExtraOptions(const std::string& scope, common::StringsList& options) const
{
    if (!m_externalRef.empty()) {
        options.push_back("TExtraOpts...");
    }

    if (!scope.empty()) {
        options.push_back("typename " + scope + common::nameToClassCopy(name()));
    }
}

std::string Field::getCustomRead() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomWrite() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomLength() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomValid() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomRefresh() const
{
    // TODO: implement
    return common::emptyString();
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

} // namespace commsdsl2comms
