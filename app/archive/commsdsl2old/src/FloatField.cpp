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

#include "FloatField.h"

#include <type_traits>
#include <cmath>
#include <limits>
#include <map>
#include <set>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2old
{

namespace
{

const std::string ClassTemplate(
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::FloatValue<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::FloatValue<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#FIELD_TYPE#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    /// @brief Re-definition of the value type.\n"
    "    using ValueType = typename Base::ValueType;\n\n"
    "    #^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
    "    #^#CONSTRUCTOR#$#\n"
    "    #^#PUBLIC#$#\n"
    "    #^#HAS_SPECIALS#$#\n"
    "    #^#SPECIALS#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "    #^#SPECIAL_NAMES_MAP#$#\n"
    "    #^#DISPLAY_DECIMALS#$#\n"
    "#^#PROTECTED#$#\n"
    "#^#PRIVATE#$#\n"
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

const std::string ValueComparisonTemplate =
    "Base::value() == #^#MIN#$#";

const std::string VersionConditionTemplate =
    "if ((#^#MIN_VERSION#$# <= Base::getVersion()) &&\n"
    "    (Base::getVersion() < #^#MAX_VERSION#$#)) {\n"
    "    #^#CONDITIONS#$#\n"
    "}\n";

const std::string FromVersionConditionTemplate =
    "if (#^#MIN_VERSION#$# <= Base::getVersion()) {\n"
    "    #^#CONDITIONS#$#\n"
    "}\n";

const std::string UntilVersionConditionTemplate =
    "if (Base::getVersion() < #^#MAX_VERSION#$#) {\n"
    "    #^#CONDITIONS#$#\n"
    "}\n";


bool isLimit(double val)
{
    return std::isnan(val) || std::isinf(val);
}

std::string limitToString(double val)
{
    static const std::string Prefix("std::numeric_limits<ValueType>::");
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

double getLowest(commsdsl::parse::FloatField::Type type)
{
    if (type == commsdsl::parse::FloatField::Type::Float) {
        return double(std::numeric_limits<float>::lowest());
    }

    return std::numeric_limits<double>::lowest();
}

double getMax(commsdsl::parse::FloatField::Type type)
{
    if (type == commsdsl::parse::FloatField::Type::Float) {
        return double(std::numeric_limits<float>::max());
    }

    return std::numeric_limits<double>::max();
}


std::string valueToString(double val, commsdsl::parse::FloatField::Type type)
{
    if (isLimit(val)) {
        return limitToString(val);
    }

    auto lowest = getLowest(type);
    if (std::abs(lowest - val) < std::numeric_limits<double>::epsilon()) {
        return "std::numeric_limits<ValueType>::lowest()";
    }

    auto max = getMax(type);
    if (std::abs(max - val) < std::numeric_limits<double>::epsilon()) {
        return "std::numeric_limits<ValueType>::max()";
    }

    return "static_cast<ValueType>(" + std::to_string(val) + ")";
}

std::string cmpToString(double val, commsdsl::parse::FloatField::Type type)
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

    return "std::abs(Base::value() - " + valueToString(val, type) + ") < std::numeric_limits<ValueType>::epsilon()";
}

void addCondition(common::StringsList& condList, std::string&& str)
{
    common::ReplacementMap repl;
    repl.insert(std::make_pair("COND", std::move(str)));
    condList.push_back(common::processTemplate(ConditionTemplate, repl));
}

void addRangeComparison(
    common::StringsList& condList,
    double min,
    double max,
    commsdsl::parse::FloatField::Type type)
{
    auto* templ = &RangeComparisonTemplate;
    if (min == max) {
        templ = &ValueComparisonTemplate;
    }
    common::ReplacementMap repl;
    repl.insert(std::make_pair("MIN", valueToString(min, type)));
    repl.insert(std::make_pair("MAX", valueToString(max, type)));
    addCondition(condList, common::processTemplate(*templ, repl));
}



} // namespace

bool FloatField::prepareImpl()
{
    auto obj = floatFieldDslObj();
    auto& specials = obj.specialValues();
    m_specials.assign(specials.begin(), specials.end());
    std::sort(
        m_specials.begin(), m_specials.end(),
        [](auto& elem1, auto& elem2)
        {
            auto compNames =
                [&elem1, &elem2]()
                {
                    return elem1.first < elem2.first;
                };

            if (std::isnan(elem1.second.m_value)) {
                if (std::isnan(elem2.second.m_value)) {
                    return compNames();
                }

                return false;
            }

            if (std::isnan(elem2.second.m_value)) {
                return true;
            }

            auto isPositiveInf =
                [](double val)
                {
                    return std::isinf(val) && (val > 0.0);
                };

            if (isPositiveInf(elem1.second.m_value)) {
                if (isPositiveInf(elem2.second.m_value)) {
                    return compNames();
                }

                return false;
            }

            auto isNegativeInf =
                [](double val)
                {
                    return std::isinf(val) && (val < 0.0);
                };

            if (isNegativeInf(elem1.second.m_value)) {
                if (isNegativeInf(elem2.second.m_value)) {
                    return compNames();
                }

                return true;
            }

            if (elem1.second.m_value != elem2.second.m_value) {
                return elem1.second.m_value < elem2.second.m_value;
            }

            return compNames();
        });
    return true;
}

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

void FloatField::updateIncludesCommonImpl(IncludesList& includes) const
{
    bool hasLimits =
        std::any_of(
            m_specials.begin(), m_specials.end(),
            [](auto& s)
            {
                return isLimit(s.second.m_value);
            });

    if (hasLimits) {
        common::mergeInclude("<limits>", includes);
    }
}

std::string FloatField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    auto adjScope = adjustScopeWithNamespace(scope);
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("CONSTRUCTOR", getConstructor()));
    replacements.insert(std::make_pair("NAME", getNameCommonWrapFunc(adjScope)));
    replacements.insert(std::make_pair("HAS_SPECIALS", getHasSpecialsFunc(adjScope)));
    replacements.insert(std::make_pair("SPECIALS", getSpecials(adjScope)));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));
    replacements.insert(std::make_pair("DISPLAY_DECIMALS", getDisplayDecimals()));

    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["FIELD_TYPE"] += ',';
    }

    if (!m_specials.empty()) {
        replacements.insert(std::make_pair("SPECIAL_VALUE_NAMES_MAP_DEFS", getSpecialNamesMapDefs(adjScope)));
        replacements.insert(std::make_pair("SPECIAL_NAMES_MAP", getSpacialNamesMapFunc(adjScope)));
    }    

    return common::processTemplate(ClassTemplate, replacements);
}

std::string FloatField::getPluginPropertiesImpl(bool serHiddenParam) const
{
    static_cast<void>(serHiddenParam);
    common::StringsList props;
    auto obj = floatFieldDslObj();
    auto decimals = obj.displayDecimals();
    if (decimals != 0U) {
        props.push_back(".decimals(" + common::numToString(decimals) + ")");
    }

    do {
        if (!obj.displaySpecials()) {
            break;
        }

        if (m_specials.empty()) {
            break;
        }

        auto addSpecDisplayNameFunc =
            [&props](double val, const std::string& name, const std::string& displayName)
            {
                std::string valStr;
                if (std::isnan(val)) {
                    valStr = "std::numeric_limits<double>::quiet_NaN()";
                }
                else if (std::isinf(val)) {
                    valStr = "std::numeric_limits<double>::infinity()";
                    if (val < 0.0) {
                        valStr = '-' + valStr;
                    }
                }
                else {
                    valStr = std::to_string(val);
                }

                auto* nameToAdd = &displayName;
                if (nameToAdd->empty()) {
                    nameToAdd = &name;
                }

                props.push_back(".addSpecial(\"" + *nameToAdd + "\", " + valStr + ")");
            };

        for (auto& s : m_specials) {
            addSpecDisplayNameFunc(s.second.m_value, s.first, s.second.m_displayName);
        }
        break;

        // The code below removes duplicates, but probably not needed.
//        if (!obj.hasNonUniqueSpecials()) {
//            for (auto& s : specials) {
//                addSpecDisplayNameFunc(s.second.m_value, s.first, s.second.m_displayName);
//            }
//            break;
//        }

//        using SpecialsDisplayMap = std::multimap<double, std::pair<const std::string*, const std::string*> >;
//        using SpecialsValuesSet = std::set<double>;
//        using SpecialsNanList = std::vector<std::pair<const std::string*, const std::string*> >;

//        SpecialsDisplayMap displayMap;
//        SpecialsValuesSet specialsSet;
//        SpecialsNanList specialNans;
//        for (auto& s : specials) {
//            if (std::isnan(s.second.m_value)) {
//                specialNans.push_back(std::make_pair(&s.first, &s.second.m_displayName));
//                continue;
//            }

//            specialsSet.insert(s.second.m_value);
//            displayMap.insert(std::make_pair(s.second.m_value, std::make_pair(&s.first, &s.second.m_displayName)));
//        }

//        if (!specialNans.empty()) {
//            auto nanIter =
//                std::find_if(
//                    specialNans.begin(), specialNans.end(),
//                    [](auto& elem)
//                    {
//                        return !elem.second->empty();
//                    });

//            if (nanIter == specialNans.end()) {
//                nanIter = specialNans.begin();
//            }

//            assert(nanIter != specialNans.end());
//            addSpecDisplayNameFunc(std::numeric_limits<double>::quiet_NaN(), *nanIter->first, *nanIter->second);
//        }

//        for (auto val : specialsSet) {
//            auto mapIter = displayMap.equal_range(val);
//            assert(mapIter.first != mapIter.second);
//            auto iter =
//                std::find_if(
//                    mapIter.first, mapIter.second,
//                    [](auto& elem)
//                    {
//                        return !elem.second.second->empty(); // displayName is not empty
//                    });
//            if (iter == mapIter.second) {
//                iter = mapIter.first;
//            }

//            addSpecDisplayNameFunc(val, *(iter->second.first), *(iter->second.second));
//        }

    } while (false);

    if (props.empty()) {
        return common::emptyString();
    }
    return common::listToString(props, "\n", common::emptyString());

}

std::string FloatField::getCommonDefinitionImpl(const std::string& fullScope) const
{
    auto obj = floatFieldDslObj();
    common::StringsList specialsList;
    for (auto& s : m_specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string SpecialTempl(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return #^#SPEC_VAL#$#;\n"
            "}\n\n"
        );

        std::string specVal;
        specVal = valueToString(s.second.m_value, obj.type());

        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = common::makeMultilineCopy(desc);
            ba::replace_all(desc, "\n", "\n///     ");
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("SPEC_NAME", s.first));
        replacements.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        replacements.insert(std::make_pair("SPEC_VAL", std::move(specVal)));
        replacements.insert(std::make_pair("SPECIAL_DOC", std::move(desc)));

        specialsList.push_back(common::processTemplate(SpecialTempl, replacements));
    }

    static const std::string Templ =
        "/// @brief Common types and functions for\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "struct #^#NAME#$#Common\n"
        "{\n"
        "    /// @brief Re-definition of the value type used by\n"
        "    ///     #^#SCOPE#$# field.\n"
        "    using ValueType = #^#VALUE_TYPE#$#;\n\n"
        "    #^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
        "    #^#NAME_FUNC#$#\n"
        "    #^#HAS_SPECIAL_FUNC#$#\n"
        "    #^#SPECIALS#$#\n"
        "    #^#SPECIAL_NAMES_MAP#$#\n"
        "};\n";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("NAME", common::nameToClassCopy(name())));
    repl.insert(std::make_pair("SCOPE", fullScope));
    repl.insert(std::make_pair("VALUE_TYPE", getFieldType()));
    repl.insert(std::make_pair("NAME_FUNC", getCommonNameFunc(fullScope)));
    repl.insert(std::make_pair("HAS_SPECIAL_FUNC", getHasSpecialsFunc()));
    if (!specialsList.empty()) {
        repl.insert(std::make_pair("SPECIAL_VALUE_NAMES_MAP_DEFS", getSpecialNamesMapDefs()));
        repl.insert(std::make_pair("SPECIALS", common::listToString(specialsList, "\n", "\n")));
        repl.insert(std::make_pair("SPECIAL_NAMES_MAP", getSpacialNamesMapFunc()));
    }
    return common::processTemplate(Templ, repl);
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
    static_assert(TypeMapSize == static_cast<std::size_t>(commsdsl::parse::FloatField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(obj.type());
    if (TypeMapSize <= idx) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
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

std::string FloatField::getSpecialNamesMapDefs(const std::string& scope) const
{
    static const std::string Templ = 
        "/// @brief Single special value name info entry.\n"
        "using SpecialNameInfo = #^#INFO_DEF#$#;\n\n"
        "/// @brief Type returned from @ref specialNamesMap() member function.\n"
        "/// @details The @b first value of the pair is pointer to the map array,\n"
        "///     The @b second value of the pair is the size of the array.\n"
        "using SpecialNamesMapInfo = #^#MAP_DEF#$#;\n";

    common::ReplacementMap repl;
    if (scope.empty()) {
        repl.insert(std::make_pair("INFO_DEF", getCommonSpecialNameInfoDef()));
        repl.insert(std::make_pair("MAP_DEF", getCommonSpecialNamesMapDef()));
    }
    else {
        repl.insert(std::make_pair("INFO_DEF", getSpecialNameInfoDef(scope)));
        repl.insert(std::make_pair("MAP_DEF", getSpecialNamesMapDef(scope)));
    }
    return common::processTemplate(Templ, repl);
}

const std::string& FloatField::getCommonSpecialNameInfoDef()
{
    static const std::string Str = "std::pair<ValueType, const char*>";
    return Str;
}

const std::string& FloatField::getCommonSpecialNamesMapDef()
{
    static const std::string Str = "std::pair<const SpecialNameInfo*, std::size_t>";
    return Str;
}


std::string FloatField::getSpecialNameInfoDef(const std::string& scope) const
{
    static const std::string Temp = 
        "#^#SCOPE#$##^#CLASS_NAME#$#Common::SpecialNameInfo";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl);   
}

std::string FloatField::getSpecialNamesMapDef(const std::string& scope) const
{
    static const std::string Temp = 
        "#^#SCOPE#$##^#CLASS_NAME#$#Common::SpecialNamesMapInfo";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl);    
}

std::string FloatField::getHasSpecialsFunc(const std::string& scope) const
{
    static const std::string Temp = 
        "/// @brief Compile time detection of special values presence.\n"
        "static constexpr bool hasSpecials()\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    std::string body;
    if (scope.empty()) {
        body = getHasSpecialsFuncCommonBody();
    }
    else {
        body = getHasSpecialsFuncBody(scope);
    }

    common::ReplacementMap repl;
    repl.insert(std::make_pair("BODY", std::move(body)));
    return common::processTemplate(Temp, repl);
}

std::string FloatField::getHasSpecialsFuncCommonBody() const
{
    static const std::string Temp = 
        "return #^#RESULT#$#;";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("RESULT", m_specials.empty() ? "false" : "true"));
    return common::processTemplate(Temp, repl);
}

std::string FloatField::getHasSpecialsFuncBody(const std::string& scope) const
{
    static const std::string Temp = 
        "return #^#SCOPE#$##^#CLASS_NAME#$#Common::hasSpecials();";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl);   
}

std::string FloatField::getSpecials(const std::string& scope) const
{
    auto obj = floatFieldDslObj();
    std::string result;
    for (auto& s : m_specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        if (!result.empty()) {
            result += '\n';
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return #^#SCOPE#$##^#CLASS_NAME#$#Common::value#^#SPEC_ACC#$#();\n"
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
        replacements.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
        replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
        replacements.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        replacements.insert(std::make_pair("SPEC_CMP", cmpToString(s.second.m_value, type)));
        replacements.insert(std::make_pair("SPECIAL_DOC", std::move(desc)));

        result += common::processTemplate(Templ, replacements);
    }
    return result;
}

std::string FloatField::getSpacialNamesMapFunc(const std::string& scope) const
{
    static const std::string Templ = 
        "/// @brief Retrieve map of special value names\n"
        "static SpecialNamesMapInfo specialNamesMap()\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    std::string body;
    if (scope.empty()) {
        body = getSpacialNamesMapFuncCommonBody();
    }
    else {
        body = getSpacialNamesMapFuncBody(scope);
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("BODY", std::move(body)));
    return common::processTemplate(Templ, replacements);
}

std::string FloatField::getSpacialNamesMapFuncCommonBody() const
{
    common::StringsList specialInfos;
    for (auto& s : m_specials) {
        static const std::string SpecTempl = 
            "std::make_pair(value#^#SPEC_ACC#$#(), \"#^#SPEC_NAME#$#\")";

        common::ReplacementMap specRepl;
        specRepl.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        specRepl.insert(std::make_pair("SPEC_NAME", s.first));
        specialInfos.push_back(common::processTemplate(SpecTempl, specRepl));
    }

    assert(!specialInfos.empty());
    static const std::string Templ = 
        "static const SpecialNameInfo Map[] = {\n"
        "    #^#INFOS#$#\n"
        "};\n"
        "static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "return std::make_pair(&Map[0], MapSize);\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("INFOS", common::listToString(specialInfos, ",\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string FloatField::getSpacialNamesMapFuncBody(const std::string& scope) const
{
    static const std::string Temp = 
        "return #^#SCOPE#$##^#CLASS_NAME#$#Common::specialNamesMap();";

    common::ReplacementMap repl;
    repl.insert(std::make_pair("SCOPE", scopeForCommon(scope)));
    repl.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    return common::processTemplate(Temp, repl); 
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

std::string FloatField::getDisplayDecimals() const
{
    auto obj = floatFieldDslObj();
    auto decimals = obj.displayDecimals();

    static const std::string Templ = 
        "/// @brief Requested number of digits after decimal point when value\n"
        "///     is displayed.\n"
        "static constexpr unsigned displayDecimals()\n"
        "{\n"
        "    return #^#DISPLAY_DECIMALS#$#;\n"
        "}";
        
    common::ReplacementMap repl;
    repl.insert(std::make_pair("DISPLAY_DECIMALS", common::numToString(decimals)));
    return common::processTemplate(Templ, repl);
}

FloatField::StringsList FloatField::getVersionBasedConditions() const
{
    auto obj = floatFieldDslObj();
    auto validRanges = obj.validRanges(); // copy

    std::sort(
        validRanges.begin(), validRanges.end(),
        [](auto& e1, auto& e2)
        {
            if (e1.m_sinceVersion != e2.m_sinceVersion) {
                return e1.m_sinceVersion < e2.m_sinceVersion;
            }

            if (e1.m_deprecatedSince != e2.m_deprecatedSince) {
                return e1.m_deprecatedSince > e2.m_deprecatedSince;
            }

            if (std::isnan(e2.m_min)) {
                return !std::isnan(e1.m_min);
            }

            if (std::isnan(e1.m_min)) {
                return false;
            }

            if (e1.m_min != e2.m_min) {
                return e1.m_min < e2.m_min;
            }

            return e1.m_max < e2.m_max;
        });


    auto minVersion = obj.sinceVersion();
    auto maxVersion = obj.deprecatedSince();
    auto verDepIter =
        std::find_if(
            validRanges.begin(), validRanges.end(),
            [minVersion, maxVersion](auto& elem)
            {
                return ((minVersion < elem.m_sinceVersion) ||
                        (elem.m_deprecatedSince < maxVersion));
            });

    auto type = obj.type();

    StringsList conditions;
    for (auto iter = validRanges.begin(); iter != verDepIter; ++iter) {
        if (isLimit(iter->m_min)) {
            addCondition(conditions, cmpToString(iter->m_min, type));
            continue;
        }

        addRangeComparison(conditions, iter->m_min, iter->m_max, type);
    }

    while (verDepIter != validRanges.end()) {
        auto fromVersion = verDepIter->m_sinceVersion;
        auto untilVersion = verDepIter->m_deprecatedSince;
        assert(minVersion <= fromVersion);
        assert((minVersion < fromVersion) || (untilVersion < maxVersion));

        auto nextIter =
            std::find_if(
                verDepIter, validRanges.end(),
                [fromVersion, untilVersion](auto& elem)
                {
                    return (elem.m_sinceVersion != fromVersion) ||
                           (elem.m_deprecatedSince != untilVersion);
                });

        StringsList innerConditions;
        for (auto iter = verDepIter; iter != nextIter; ++iter) {
            if (isLimit(iter->m_min)) {
                addCondition(innerConditions, cmpToString(iter->m_min, type));
                continue;
            }

            addRangeComparison(innerConditions, iter->m_min, iter->m_max, type);
        }

        auto* templ = &VersionConditionTemplate;
        if (fromVersion == 0) {
            assert(untilVersion < commsdsl::parse::Protocol::notYetDeprecated());
            templ = &UntilVersionConditionTemplate;
        }
        else if (commsdsl::parse::Protocol::notYetDeprecated() <= untilVersion) {
            templ = &FromVersionConditionTemplate;
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("MIN_VERSION", common::numToString(fromVersion)));
        replacements.insert(std::make_pair("MAX_VERSION", common::numToString(untilVersion)));
        replacements.insert(std::make_pair("CONDITIONS", common::listToString(innerConditions, "\n", common::emptyString())));
        conditions.push_back(common::processTemplate(*templ, replacements));
        verDepIter = nextIter;
    }

    return conditions;
}

FloatField::StringsList FloatField::getNormalConditions() const
{
    common::StringsList conditions;
    auto obj = floatFieldDslObj();
    auto type = obj.type();

    auto& validRanges = obj.validRanges();
    for (auto& r : validRanges) {
        if (isLimit(r.m_min)) {
            addCondition(conditions, cmpToString(r.m_min, type));
            continue;
        }

        addRangeComparison(conditions, r.m_min, r.m_max, type);
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
        list.push_back("comms::option::def::VersionStorage");
    }
}

void FloatField::checkValidityOpt(FloatField::StringsList& list) const
{
    auto obj = floatFieldDslObj();

    if (!obj.validRanges().empty()) {
        list.push_back("comms::option::def::InvalidByDefault");
    }
}

} // namespace commsdsl2old