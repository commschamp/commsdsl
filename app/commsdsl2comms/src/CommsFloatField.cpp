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

#include "CommsFloatField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

namespace 
{

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

double getLowest(commsdsl::parse::ParseFloatField::Type type)
{
    if (type == commsdsl::parse::ParseFloatField::Type::Float) {
        return double(std::numeric_limits<float>::lowest());
    }

    return std::numeric_limits<double>::lowest();
}

double getMax(commsdsl::parse::ParseFloatField::Type type)
{
    if (type == commsdsl::parse::ParseFloatField::Type::Float) {
        return double(std::numeric_limits<float>::max());
    }

    return std::numeric_limits<double>::max();
}

std::string valueToString(double val, commsdsl::parse::ParseFloatField::Type type)
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

std::string cmpToString(double val, commsdsl::parse::ParseFloatField::Type type)
{
    if (std::isnan(val)) {
        return "std::isnan(Base::getValue())";
    }

    if (std::isinf(val)) {
        static const std::string Templ(
            "(#^#COMP#$#) &&\n"
            "(std::isinf(Base::getValue()))");

        std::string comp;
        if (val < 0) {
            comp = "Base::getValue() < 0";
        }
        else {
            comp = "0 < Base::getValue()";
        }
        util::ReplacementMap repl = {
            {"COMP", std::move(comp)}
        };
        return util::processTemplate(Templ, repl);
    }

    return "std::abs(Base::getValue() - " + valueToString(val, type) + ") < std::numeric_limits<ValueType>::epsilon()";
}

const std::string& specialNamesMapTempl()
{
    static const std::string Templ = 
        "/// @brief Single special value name info entry.\n"
        "using SpecialNameInfo = #^#INFO_DEF#$#;\n\n"
        "/// @brief Type returned from @ref specialNamesMap() member function.\n"
        "/// @details The @b first value of the pair is pointer to the map array,\n"
        "///     The @b second value of the pair is the size of the array.\n"
        "using SpecialNamesMapInfo = #^#MAP_DEF#$#;\n";

    return Templ;
}

const std::string& hasSpecialsFuncTempl()
{
    static const std::string Templ = 
        "/// @brief Compile time detection of special values presence.\n"
        "static constexpr bool hasSpecials()\n"
        "{\n"
        "    return #^#VALUE#$#;\n"
        "}\n";

    return Templ;
}

void addCondition(util::StringsList& condList, std::string&& str)
{
    static const std::string Templ = 
        "if (#^#COND#$#) {\n"
        "    return true;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"COND", std::move(str)},
    };

    condList.push_back(util::processTemplate(Templ, repl));
}

void addRangeComparison(
    util::StringsList& condList,
    double min,
    double max,
    commsdsl::parse::ParseFloatField::Type type)
{
    static const std::string RangeComparisonTemplate =
        "(#^#MIN#$# <= Base::getValue()) &&\n"
        "(Base::getValue() <= #^#MAX#$#)";

    static const std::string ValueComparisonTemplate =
        "Base::getValue() == #^#MIN#$#";
            
    auto* templ = &RangeComparisonTemplate;
    if (min == max) {
        templ = &ValueComparisonTemplate;
    }

    util::ReplacementMap repl = {
        {"MIN", valueToString(min, type)},
        {"MAX", valueToString(max, type)},
    };

    addCondition(condList, util::processTemplate(*templ, repl));
}

} // namespace 
    

CommsFloatField::CommsFloatField(
    CommsGenerator& generator, 
    commsdsl::parse::ParseField dslObj, 
    commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsFloatField::prepareImpl()
{
    return Base::prepareImpl() && commsPrepare();
}

bool CommsFloatField::writeImpl() const
{
    return commsWrite();
}

CommsFloatField::IncludesList CommsFloatField::commsCommonIncludesImpl() const
{
    IncludesList result;

    auto& specials = specialsSortedByValue();
    bool hasLimits =
        std::any_of(
            specials.begin(), specials.end(),
            [](auto& s)
            {
                return isLimit(s.second.m_value);
            });

    if (hasLimits) {
        result.push_back("<limits>");
    }

    return result;
}

std::string CommsFloatField::commsCommonCodeBodyImpl() const
{
    static const std::string Templ = 
        "/// @brief Re-definition of the value type used by\n"
        "///     #^#SCOPE#$# field.\n"
        "using ValueType = #^#VALUE_TYPE#$#;\n\n"
        "#^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
        "#^#NAME_FUNC#$#\n"
        "#^#HAS_SPECIAL_FUNC#$#\n"
        "#^#SPECIALS#$#\n"
        "#^#SPECIAL_NAMES_MAP#$#\n"    
    ;

    auto& gen = generator();
    auto dslObj = floatDslObj();
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, gen)},
        {"VALUE_TYPE", comms::cppFloatTypeFor(dslObj.parseType())},
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"HAS_SPECIAL_FUNC", commsCommonHasSpecialsFuncCodeInternal()}
    };

    if (!specialsSortedByValue().empty()) {
        repl.insert({
            {"SPECIAL_VALUE_NAMES_MAP_DEFS", commsCommonValueNamesMapCodeInternal()},
            {"SPECIALS", commsCommonSpecialsCodeInternal()},
            {"SPECIAL_NAMES_MAP", commsCommonSpecialNamesMapCodeInternal()},
        });
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsFloatField::commsDefConstructCodeImpl() const
{
    auto obj = floatDslObj();
    double defaultValue = obj.parseDefaultValue();
    if (defaultValue == 0) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "Base::setValue(#^#VAL#$#);\n";

    util::ReplacementMap repl = {
        {"VAL", valueToString(defaultValue, obj.parseType())}
    };

    return util::processTemplate(Templ, repl);
}

CommsFloatField::IncludesList CommsFloatField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/FloatValue.h"
    };

    bool hasLimits = false;
    do {
        auto obj = floatDslObj();
        hasLimits = isLimit(obj.parseDefaultValue());
        if (hasLimits) {
            break;
        }

        for (auto& r : obj.parseValidRanges()) {
            hasLimits = isLimit(r.m_min);
            if (hasLimits) {
                assert(isLimit(r.m_max));
                break;
            }

            assert(!isLimit(r.m_max));
        }

    } while (false);

    if (hasLimits) {
        result.insert(result.end(), {
            "<cmath>",
            "<limits>"
        });
    }    

    return result;
}

std::string CommsFloatField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::FloatValue<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    #^#FIELD_TYPE#$##^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";    

    auto& gen = generator();
    auto dslObj = floatDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.schemaOf(*this).mainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(dslObj.parseEndian())},
        {"FIELD_TYPE", comms::cppFloatTypeFor(dslObj.parseType())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);     
}

std::string CommsFloatField::commsDefPublicCodeImpl() const
{
    static const std::string Templ = 
        "/// @brief Re-definition of the value type.\n"
        "using ValueType = typename Base::ValueType;\n\n"
        "#^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
        "#^#HAS_SPECIALS#$#\n"
        "#^#SPECIALS#$#\n"
        "#^#SPECIAL_NAMES_MAP#$#\n"
        "#^#DISPLAY_DECIMALS#$#\n"
    ;

    util::ReplacementMap repl = {
        {"SPECIAL_VALUE_NAMES_MAP_DEFS", commsDefValueNamesMapCodeInternal()},
        {"HAS_SPECIALS", commsDefHasSpecialsFuncCodeInternal()},
        {"SPECIALS", commsDefSpecialsCodeInternal()},
        {"SPECIAL_NAMES_MAP", commsDefSpecialNamesMapCodeInternal()},
        {"DISPLAY_DECIMALS", commsDefDisplayDecimalsCodeInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsFloatField::commsDefValidFuncBodyImpl() const
{
    auto obj = floatDslObj();

    auto& validRanges = obj.parseValidRanges();
    if (validRanges.empty()) {
        return strings::emptyString();
    }

    bool validCheckVersion =
        generator().schemaOf(*this).versionDependentCode() &&
        obj.parseValidCheckVersion();

    StringsList conditions;
    do {
        if (!validCheckVersion) {
            conditions = commsValidNormalConditionsInternal();
            break;
        }

        auto minVersion = obj.parseSinceVersion();
        auto maxVersion = obj.parseDeprecatedSince();
        for (auto& r : validRanges) {
            if ((minVersion < r.m_sinceVersion) ||
                (r.m_deprecatedSince < maxVersion)) {
                conditions = commsValidVersionBasedConditionsInternal();
                break;
            }
        }

        if (!conditions.empty()) {
            break;
        }

        conditions = commsValidNormalConditionsInternal();
    } while (false);

    assert(!conditions.empty());

    static const std::string Templ = 
        "if (Base::valid()) {\n"
        "    return true;\n"
        "}\n\n"
        "#^#CONDITIONS#$#\n"
        "return false;";

    util::ReplacementMap repl = {
        {"CONDITIONS", util::strListToString(conditions, "\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

bool CommsFloatField::commsIsVersionDependentImpl() const
{
    assert(generator().schemaOf(*this).versionDependentCode());
    auto obj = floatDslObj();
    if (!obj.parseValidCheckVersion()) {
        return false;
    }

    auto& validRanges = obj.parseValidRanges();
    if (validRanges.empty()) {
        return false;
    }

    unsigned minVersion = obj.parseSinceVersion();
    unsigned maxVersion = obj.parseDeprecatedSince();
    auto iter =
        std::find_if(
            validRanges.begin(), validRanges.end(),
            [minVersion, maxVersion](auto& elem)
            {
                return
                    (minVersion < elem.m_sinceVersion) ||
                    (elem.m_deprecatedSince < maxVersion);
            });    

    return (iter != validRanges.end());    
}

bool CommsFloatField::commsVerifyInnerRefImpl(const std::string& refStr) const
{
    auto obj = floatDslObj();
    auto& specials = obj.parseSpecialValues();
    return (specials.find(refStr) != specials.end());
}

std::string CommsFloatField::commsCommonHasSpecialsFuncCodeInternal() const
{
    auto& specials = specialsSortedByValue();    
    util::ReplacementMap repl = {
        {"VALUE", util::boolToString(!specials.empty())}
    };

    return util::processTemplate(hasSpecialsFuncTempl(), repl);
}

std::string CommsFloatField::commsCommonValueNamesMapCodeInternal() const
{
    auto& specials = specialsSortedByValue();    
    if (specials.empty()) {
        return strings::emptyString();
    }

    util::ReplacementMap repl = {
        {"INFO_DEF", "std::pair<ValueType, const char*>"},
        {"MAP_DEF", "std::pair<const SpecialNameInfo*, std::size_t>"}
    };

    return util::processTemplate(specialNamesMapTempl(), repl);
}

std::string CommsFloatField::commsCommonSpecialsCodeInternal() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    util::StringsList specialsList;
    for (auto& s : specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return #^#SPEC_VAL#$#;\n"
            "}\n"
        );

        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = util::strMakeMultiline(desc);
            desc = util::strReplace(desc, "\n", "\n///     ");
        }

        util::ReplacementMap repl = {
            {"SPEC_NAME", s.first},
            {"SPEC_ACC", comms::className(s.first)},
            {"SPEC_VAL", valueToString(s.second.m_value, floatDslObj().parseType())},
            {"SPECIAL_DOC", std::move(desc)},
        };

        specialsList.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(specialsList, "\n", "\n");
}

std::string CommsFloatField::commsCommonSpecialNamesMapCodeInternal() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Retrieve map of special value names\n"
        "static SpecialNamesMapInfo specialNamesMap()\n"
        "{\n"
        "    static const SpecialNameInfo Map[] = {\n"
        "        #^#INFOS#$#\n"
        "    };\n"
        "    static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "    return std::make_pair(&Map[0], MapSize);\n"
        "}\n";    

    util::StringsList specialInfos;
    for (auto& s : specials) {
        static const std::string SpecTempl = 
            "std::make_pair(value#^#SPEC_ACC#$#(), \"#^#SPEC_NAME#$#\")";

        util::ReplacementMap specRepl = {
            {"SPEC_ACC", comms::className(s.first)},
            {"SPEC_NAME", s.first}
        };
        specialInfos.push_back(util::processTemplate(SpecTempl, specRepl));
    }

    util::ReplacementMap repl {
        {"INFOS", util::strListToString(specialInfos, ",\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsFloatField::commsDefSpecialsCodeInternal() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    auto obj = floatDslObj();
    auto type = obj.parseType();

    util::StringsList specialsList;
    
    for (auto& s : specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "/// @see @ref #^#COMMON#$#::value#^#SPEC_ACC#$#().\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return #^#COMMON#$#::value#^#SPEC_ACC#$#();\n"
            "}\n\n"
            "/// @brief Check the value is equal to special @ref value#^#SPEC_ACC#$#().\n"
            "bool is#^#SPEC_ACC#$#() const\n"
            "{\n"
            "    return (#^#SPEC_CMP#$#);\n"
            "}\n\n"
            "/// @brief Assign special value @ref value#^#SPEC_ACC#$#() to the field.\n"
            "void set#^#SPEC_ACC#$#()\n"
            "{\n"
            "    Base::setValue(value#^#SPEC_ACC#$#());\n"
            "}\n"            
        );

        util::ReplacementMap repl = {
            {"SPEC_NAME", s.first},
            {"SPEC_ACC", comms::className(s.first)},
            {"COMMON", comms::commonScopeFor(*this, generator())},
            {"SPEC_CMP", cmpToString(s.second.m_value, type)}
        };

        specialsList.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(specialsList, "\n", "");
}

std::string CommsFloatField::commsDefSpecialNamesMapCodeInternal() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Retrieve map of special value names\n"
        "static SpecialNamesMapInfo specialNamesMap()\n"
        "{\n"
        "    return #^#COMMON#$#::specialNamesMap();\n"
        "}\n";    

    util::ReplacementMap repl {
        {"COMMON", comms::commonScopeFor(*this, generator())}
    };

    return util::processTemplate(Templ, repl);    
}

std::string CommsFloatField::commsDefDisplayDecimalsCodeInternal() const
{
    static const std::string Templ = 
        "/// @brief Requested number of digits after decimal point when value\n"
        "///     is displayed.\n"
        "static constexpr unsigned displayDecimals()\n"
        "{\n"
        "    return #^#DISPLAY_DECIMALS#$#;\n"
        "}";
        
    util::ReplacementMap repl = {
        {"DISPLAY_DECIMALS", util::numToString(floatDslObj().parseDisplayDecimals())}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsFloatField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddUnitsOptInternal(opts);
    commsAddVersionOptInternal(opts);
    commsAddInvalidOptInternal(opts);

    return util::strListToString(opts, ",\n", "");
}

std::string CommsFloatField::commsDefValueNamesMapCodeInternal() const
{
    auto& specials = specialsSortedByValue();    
    if (specials.empty()) {
        return strings::emptyString();
    }

    auto scope = comms::commonScopeFor(*this, generator());

    util::ReplacementMap repl = {
        {"INFO_DEF", scope + "::SpecialNameInfo"},
        {"MAP_DEF", scope + "::SpecialNamesMapInfo"}
    };

    return util::processTemplate(specialNamesMapTempl(), repl);    
}

std::string CommsFloatField::commsDefHasSpecialsFuncCodeInternal() const
{
    util::ReplacementMap repl = {
        {"VALUE", comms::commonScopeFor(*this, generator()) + "::hasSpecials()"}
    };

    return util::processTemplate(hasSpecialsFuncTempl(), repl);
}

void CommsFloatField::commsAddUnitsOptInternal(StringsList& opts) const
{
    auto obj = floatDslObj();
    auto units = obj.parseUnits();
    auto& str = comms::dslUnitsToOpt(units);
    if (!str.empty()) {
        util::addToStrList(str, opts);
    }
}

void CommsFloatField::commsAddVersionOptInternal(StringsList& opts) const
{
    auto obj = floatDslObj();
    bool validCheckVersion =
        generator().schemaOf(*this).versionDependentCode() &&
        obj.parseValidCheckVersion();

    if (!validCheckVersion) {
        return;
    }
    
    auto& validRanges = obj.parseValidRanges();
    if (validRanges.empty()) {
        return;
    }
    
    unsigned minVersion = obj.parseSinceVersion();
    unsigned maxVersion = obj.parseDeprecatedSince();
    bool versionDependent = false;
    for (auto& r : validRanges) {
        if ((minVersion < r.m_sinceVersion) ||
            (r.m_deprecatedSince < maxVersion)) {
            versionDependent = true;
        }
    }

    if (versionDependent) {
        opts.push_back("comms::option::def::VersionStorage");
    }
}

void CommsFloatField::commsAddInvalidOptInternal(StringsList& opts) const
{
    auto obj = floatDslObj();

    if (!obj.parseValidRanges().empty()) {
        opts.push_back("comms::option::def::InvalidByDefault");
    }
}

CommsFloatField::StringsList CommsFloatField::commsValidNormalConditionsInternal() const
{
    util::StringsList conditions;
    auto obj = floatDslObj();
    auto type = obj.parseType();

    auto& validRanges = obj.parseValidRanges();
    for (auto& r : validRanges) {
        if (isLimit(r.m_min)) {
            addCondition(conditions, cmpToString(r.m_min, type));
            continue;
        }

        addRangeComparison(conditions, r.m_min, r.m_max, type);
    }

    return conditions;
}

CommsFloatField::StringsList CommsFloatField::commsValidVersionBasedConditionsInternal() const
{
    auto obj = floatDslObj();
    auto validRanges = obj.parseValidRanges(); // copy

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


    auto minVersion = obj.parseSinceVersion();
    auto maxVersion = obj.parseDeprecatedSince();
    auto verDepIter =
        std::find_if(
            validRanges.begin(), validRanges.end(),
            [minVersion, maxVersion](auto& elem)
            {
                return ((minVersion < elem.m_sinceVersion) ||
                        (elem.m_deprecatedSince < maxVersion));
            });

    auto type = obj.parseType();

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

        static const std::string VersionConditionTemplate =
            "if ((#^#MIN_VERSION#$# <= Base::getVersion()) &&\n"
            "    (Base::getVersion() < #^#MAX_VERSION#$#)) {\n"
            "    #^#CONDITIONS#$#\n"
            "}\n";

        static const std::string FromVersionConditionTemplate =
            "if (#^#MIN_VERSION#$# <= Base::getVersion()) {\n"
            "    #^#CONDITIONS#$#\n"
            "}\n";

        static const std::string UntilVersionConditionTemplate =
            "if (Base::getVersion() < #^#MAX_VERSION#$#) {\n"
            "    #^#CONDITIONS#$#\n"
            "}\n";        

        auto* templ = &VersionConditionTemplate;
        if (fromVersion == 0) {
            assert(untilVersion < commsdsl::parse::ParseProtocol::parseNotYetDeprecated());
            templ = &UntilVersionConditionTemplate;
        }
        else if (commsdsl::parse::ParseProtocol::parseNotYetDeprecated() <= untilVersion) {
            templ = &FromVersionConditionTemplate;
        }

        util::ReplacementMap repl = {
            {"MIN_VERSION", util::numToString(fromVersion)},
            {"MAX_VERSION", util::numToString(untilVersion)},
            {"CONDITIONS", util::strListToString(innerConditions, "\n", "")},
        };

        conditions.push_back(util::processTemplate(*templ, repl));
        verDepIter = nextIter;
    }

    return conditions;
}

} // namespace commsdsl2comms
