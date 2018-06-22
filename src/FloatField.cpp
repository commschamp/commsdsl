#include "FloatField.h"

#include <type_traits>
#include <cmath>
#include <limits>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::FloatValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::FloatValue<\n"
    "            #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#FIELD_TYPE#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#CONSTRUCTOR#$#\n"
    "    #^#SPECIALS#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "};\n"
);

const std::string StructTemplate(
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::FloatValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#NAME#$#\n"
    "};\n"
);

const std::string ValidFuncTemplate(
    "/// @brief Custom validity check\n"
    "bool valid() const\n"
    "{\n"
    "    if (Base::valid()) {\n"
    "        return true;\n"
    "    }\n\n"
    "    #^#CONDITIONS#$#\n"
    "    return false;\n"
    "}\n");

const std::string ConditionTemplate =
    "if (#^#COND#$#) {\n"
    "    return true;\n"
    "}\n";


const std::string RangeComparisonTemplate =
    "(#^#MIN#$# <= Base::value()) &&\n"
    "(Base::value() <= #^#MAX#$#)";

bool shouldUseStruct(const common::ReplacementMap& replacements)
{
    auto hasNoValue =
        [&replacements](const std::string& val)
        {
            auto iter = replacements.find(val);
            return (iter == replacements.end()) || iter->second.empty();
        };

    return
        hasNoValue("CONSTRUCTOR") &&
        hasNoValue("SPECIALS") &&
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH");
}

bool isLimit(double val)
{
    return std::isnan(val) || std::isinf(val);
}

std::string limitToString(double val)
{
    static const std::string Prefix("std::numeric_limits<typename Base::ValueType>::");
    if (std::isnan(val)) {
        return Prefix + "quiet_NaN()";
    }

    assert(std::isinf(val));
    static const std::string Inf("infinity()");
    if (val < 0) {
        return '-' + Prefix + Inf;
    }

    return Prefix + Inf;
}

double getLowest(commsdsl::FloatField::Type type)
{
    if (type == commsdsl::FloatField::Type::Float) {
        return std::numeric_limits<float>::lowest();
    }

    return std::numeric_limits<double>::lowest();
}

double getMax(commsdsl::FloatField::Type type)
{
    if (type == commsdsl::FloatField::Type::Float) {
        return std::numeric_limits<float>::max();
    }

    return std::numeric_limits<double>::max();
}


std::string valueToString(double val, commsdsl::FloatField::Type type)
{
    if (isLimit(val)) {
        return limitToString(val);
    }

    auto lowest = getLowest(type);
    if (std::abs(lowest - val) < std::numeric_limits<double>::epsilon()) {
        return "std::numeric_limits<typename Base::ValueType>::lowest()";
    }

    auto max = getMax(type);
    if (std::abs(max - val) < std::numeric_limits<double>::epsilon()) {
        return "std::numeric_limits<typename Base::ValueType>::max()";
    }

    return "static_cast<typename Base::ValueType>(" + std::to_string(val) + ")";
}

std::string cmpToString(double val, commsdsl::FloatField::Type type)
{
    if (std::isnan(val)) {
        return "std::isnan(Base::value())";
    }

    if (std::isinf(val)) {
        static const std::string Templ(
            "(#^#COMP#$#) &&\n"
            "(std::isinf(Base::value()))");

        std::string comp;
        if (val < 0) {
            comp = "Base::value() < 0";
        }
        else {
            comp = "0 < Base::value()";
        }
        common::ReplacementMap repl;
        repl.insert(std::make_pair("COMP", std::move(comp)));
        return common::processTemplate(Templ, repl);
    }

    return "std::abs(Base::value() - " + valueToString(val, type) + ") < std::numeric_limits<typename Base::ValueType>::epsilon()";
}

} // namespace

void FloatField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/FloatValue.h",
    };

    common::mergeIncludes(List, includes);

    bool hasLimits = false;
    do {
        auto obj = floatFieldDslObj();
        hasLimits = isLimit(obj.defaultValue());
        if (hasLimits) {
            break;
        }

        for (auto& s : obj.specialValues()) {
            hasLimits = isLimit(s.second.m_value);
            if (hasLimits) {
                break;
            }
        }

        if (hasLimits) {
            break;
        }

        for (auto& r : obj.validRanges()) {
            hasLimits = isLimit(r.m_min);
            if (hasLimits) {
                assert(isLimit(r.m_max));
                break;
            }

            assert(!isLimit(r.m_max));
        }

    } while (false);

    if (hasLimits) {
        common::mergeInclude("<cmath>", includes);
        common::mergeInclude("<limits>", includes);
    }
}

std::string FloatField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(suffix)));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("CONSTRUCTOR", getConstructor()));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("SPECIALS", getSpecials()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["FIELD_TYPE"] += ',';
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string FloatField::getFieldBaseParams() const
{
    auto obj = floatFieldDslObj();
    auto endian = obj.endian();
    return getCommonFieldBaseParams(endian);
}

const std::string& FloatField::getFieldType() const
{
    auto obj = floatFieldDslObj();

    static const std::string TypeMap[] = {
        /* Float */ "float",
        /* Double */ "double",
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<decltype(TypeMapSize)>(commsdsl::FloatField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(obj.type());
    if (TypeMapSize <= idx) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    return TypeMap[idx];
}

std::string FloatField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    checkUnitsOpt(options);
    checkVersionOpt(options);
    checkValidityOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string FloatField::getConstructor() const
{
    auto obj = floatFieldDslObj();
    double defaultValue = obj.defaultValue();
    if (!isLimit(defaultValue)) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Default constructor.\n"
        "#^#CLASS_NAME#$#()\n"
        "{\n"
        "    Base::value() = #^#VAL#$#;\n"
        "}\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("VAL", valueToString(defaultValue, obj.type())));
    return common::processTemplate(Templ, replacements);
}

std::string FloatField::getSpecials() const
{
    auto obj = floatFieldDslObj();
    auto& specials = obj.specialValues();
    std::string result;
    for (auto& s : specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        if (!result.empty()) {
            result += '\n';
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr typename Base::ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return #^#SPEC_VAL#$#;\n"
            "}\n\n"
            "/// @brief Check the value is equal to special @ref value#^#SPEC_ACC#$#().\n"
            "bool is#^#SPEC_ACC#$#() const\n"
            "{\n"
            "    return (#^#SPEC_CMP#$#);\n"
            "}\n\n"
            "/// @brief Assign special value @ref value#^#SPEC_ACC#$#() to the field.\n"
            "void set#^#SPEC_ACC#$#()\n"
            "{\n"
            "    Base::value() = value#^#SPEC_ACC#$#();\n"
            "}\n"
        );


        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = common::makeMultilineCopy(desc);
            ba::replace_all(desc, "\n", "\n///     ");
        }

        auto type = obj.type();
        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("SPEC_NAME", s.first));
        replacements.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        replacements.insert(std::make_pair("SPEC_VAL", valueToString(s.second.m_value, type)));
        replacements.insert(std::make_pair("SPEC_CMP", cmpToString(s.second.m_value, type)));
        replacements.insert(std::make_pair("SPECIAL_DOC", std::move(desc)));

        result += common::processTemplate(Templ, replacements);
    }
    return result;
}

std::string FloatField::getValid() const
{
    auto custom = getCustomValid();
    if (!custom.empty()) {
        return custom;
    }

    auto obj = floatFieldDslObj();

    auto& validRanges = obj.validRanges();
    if (validRanges.empty()) {
        return common::emptyString();
    }

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    StringsList conditions;
    do {
        if (!validCheckVersion) {
            conditions = getNormalConditions();
            break;
        }

        auto minVersion = obj.sinceVersion();
        auto maxVersion = obj.deprecatedSince();
        for (auto& r : validRanges) {
            if ((minVersion < r.m_sinceVersion) ||
                (r.m_deprecatedSince < maxVersion)) {
                conditions = getVersionBasedConditions();
                break;
            }
        }

        if (!conditions.empty()) {
            break;
        }

        conditions = getNormalConditions();
    } while (false);

    assert(!conditions.empty());
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CONDITIONS", common::listToString(conditions, "\n", common::emptyString())));
    return common::processTemplate(ValidFuncTemplate, replacements);
}

FloatField::StringsList FloatField::getVersionBasedConditions() const
{
    // TODO:
    return StringsList();
}

FloatField::StringsList FloatField::getNormalConditions() const
{
    common::StringsList conditions;
    auto addCondFunc =
        [&conditions](std::string&& str)
        {
            common::ReplacementMap repl;
            repl.insert(std::make_pair("COND", std::move(str)));
            conditions.push_back(common::processTemplate(ConditionTemplate, repl));
        };

    auto obj = floatFieldDslObj();
    auto type = obj.type();

    auto addRangeCompFunc =
        [&addCondFunc, type](double min, double max)
        {
            common::ReplacementMap repl;
            repl.insert(std::make_pair("MIN", valueToString(min, type)));
            repl.insert(std::make_pair("MAX", valueToString(max, type)));
            addCondFunc(common::processTemplate(RangeComparisonTemplate, repl));
        };


    auto& validRanges = obj.validRanges();
    for (auto& r : validRanges) {
        if (isLimit(r.m_min)) {
            addCondFunc(cmpToString(r.m_min, type));
            continue;
        }

        addRangeCompFunc(r.m_min, r.m_max);
    }

    return conditions;
}

void FloatField::checkUnitsOpt(FloatField::StringsList& list) const
{
    auto obj = floatFieldDslObj();
    auto units = obj.units();
    auto& str = common::dslUnitsToOpt(units);
    if (!str.empty()) {
        list.push_back(str);
    }
}

void FloatField::checkVersionOpt(FloatField::StringsList& list) const
{
    auto obj = floatFieldDslObj();
    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        return;
    }
    
    auto& validRanges = obj.validRanges();
    if (validRanges.empty()) {
        return;
    }
    
    unsigned minVersion = obj.sinceVersion();
    unsigned maxVersion = obj.deprecatedSince();
    bool versionDependent = false;
    for (auto& r : validRanges) {
        if ((minVersion < r.m_sinceVersion) ||
            (r.m_deprecatedSince < maxVersion)) {
            versionDependent = true;
        }
    }

    if (versionDependent) {
        list.push_back("comms::option::VersionStorage");
    }
}

void FloatField::checkValidityOpt(FloatField::StringsList& list) const
{
    auto obj = floatFieldDslObj();

    if (!obj.validRanges().empty()) {
        list.push_back("comms::option::InvalidByDefault");
    }
}

} // namespace commsdsl2comms
