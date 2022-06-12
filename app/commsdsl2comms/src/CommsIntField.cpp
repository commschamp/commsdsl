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

#include "CommsIntField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

namespace 
{

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

} // namespace 
    

CommsIntField::CommsIntField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

std::string CommsIntField::commsVariantPropKeyType() const
{
    return commsDefBaseClassInternal(true);
}

std::string CommsIntField::commsVariantPropKeyValueStr() const
{
    auto obj = intDslObj();
    if (!isUnsignedType()) {
        return util::numToString(obj.defaultValue());
    }

    unsigned hexWidth = static_cast<unsigned>(obj.maxLength() * 2U);
    auto val = static_cast<std::uintmax_t>(obj.defaultValue());
    auto decValue = util::numToString(val);
    auto hexValue = util::numToString(val, hexWidth);

    static const std::string Templ = 
        "#^#DEC#$# /* #^#HEX#$# */";

    util::ReplacementMap repl = {
        {"DEC", std::move(decValue)},
        {"HEX", std::move(hexValue)},
    };
    return util::processTemplate(Templ, repl);
}

bool CommsIntField::commsVariantIsValidPropKey() const
{
    auto obj = intDslObj();
    if (!obj.isFailOnInvalid()) {
        return false;
    }

    if (obj.isPseudo()) {
        return false;
    }

    auto& validRanges = obj.validRanges();
    if (validRanges.size() != 1U) {
        return false;
    }

    auto& r = validRanges.front();
    if (r.m_min != r.m_max) {
        return false;
    }

    if (r.m_min != obj.defaultValue()) {
        return false;
    }

    return true;
}

bool CommsIntField::prepareImpl()
{
    return Base::prepareImpl() && commsPrepare();
}

bool CommsIntField::writeImpl() const
{
    return commsWrite();
}

CommsIntField::IncludesList CommsIntField::commsCommonIncludesImpl() const
{
    IncludesList list = {
        "<cstdint>"
    };

    auto& specials = specialsSortedByValue(); 
    if (!specials.empty()) {
        list.insert(list.end(),
            {
                "<type_traits>", 
                "<utility>"
            });
    }

    return list;
}

std::string CommsIntField::commsCommonCodeBodyImpl() const
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

    //auto& specials = specialsSortedByValue();

    auto& gen = generator();
    auto dslObj = intDslObj();
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, gen, true, true)},
        {"VALUE_TYPE", comms::cppIntTypeFor(dslObj.type(), dslObj.maxLength())},
        {"SPECIAL_VALUE_NAMES_MAP_DEFS", commsCommonValueNamesMapCodeInternal()},
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"HAS_SPECIAL_FUNC", commsCommonHasSpecialsFuncCodeInternal()},
        {"SPECIALS", commsCommonSpecialsCodeInternal()},
        {"SPECIAL_NAMES_MAP", commsCommonSpecialNamesMapCodeInternal()},
    };
    return util::processTemplate(Templ, repl);
}

CommsIntField::IncludesList CommsIntField::commsDefIncludesImpl() const
{
    IncludesList list = {
        "<cstdint>",
        "comms/field/IntValue.h"
    };

    return list;
}

std::string CommsIntField::commsDefBaseClassImpl() const
{
    return commsDefBaseClassInternal();
}

std::string CommsIntField::commsDefPublicCodeImpl() const
{
    static const std::string Templ = 
        "/// @brief Re-definition of the value type.\n"
        "using ValueType = typename Base::ValueType;\n\n"
        "#^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
        "#^#HAS_SPECIALS#$#\n"
        "#^#SPECIALS#$#\n"
        "#^#SPECIAL_NAMES_MAP#$#\n"
        "#^#DISPLAY_DECIMALS#$#\n";

    util::ReplacementMap repl = {
        {"SPECIAL_VALUE_NAMES_MAP_DEFS", commsDefValueNamesMapCodeInternal()},
        {"HAS_SPECIALS", commsDefHasSpecialsFuncCodeInternal()},
        {"SPECIALS", commsDefSpecialsCodeInternal()},
        {"SPECIAL_NAMES_MAP", commsDefSpecialNamesMapCodeInternal()},
        {"DISPLAY_DECIMALS", commsDefDisplayDecimalsCodeInternal()},
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsIntField::commsDefRefreshFuncBodyImpl() const
{
    if (!commsRequiresFailOnInvalidRefreshInternal()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "bool updated = Base::refresh();\n"
        "if (Base::valid()) {\n"
        "    return updated;\n"
        "};\n"
        "Base::setValue(#^#VALID_VALUE#$#);\n"
        "return true;\n";

    auto obj = intDslObj();
    auto& validRanges = obj.validRanges();    
    util::ReplacementMap repl = {
        {"VALID_VALUE", util::numToString(validRanges.front().m_min)},
    };
    return util::processTemplate(Templ, repl);    
}

std::string CommsIntField::commsDefValidFuncBodyImpl() const
{
    auto obj = intDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        return strings::emptyString();
    }

    auto validRanges = obj.validRanges(); // copy
    validRanges.erase(
        std::remove_if(
            validRanges.begin(), validRanges.end(),
            [this](auto& r)
            {
                return !this->generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince, true);
            }),
        validRanges.end());

    if (validRanges.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "if (Base::valid()) {\n"
        "    return true;\n"
        "}\n\n"
        "#^#RANGES_CHECKS#$#\n"
        "return false;\n"
        ;

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::parse::IntField::Type::Uint64) ||
        (type == commsdsl::parse::IntField::Type::Uintvar);

    std::string rangesChecks;
    for (auto& r : validRanges) {
        if (!rangesChecks.empty()) {
            rangesChecks += '\n';
        }

        static const std::string RangeTempl =
            "if (#^#COND#$#) {\n"
            "    return true;\n"
            "}\n";

        std::string minVal;
        std::string maxVal;

        if (bigUnsigned) {
            minVal = util::numToString(static_cast<std::uintmax_t>(r.m_min));
            maxVal = util::numToString(static_cast<std::uintmax_t>(r.m_max));
        }
        else {
            minVal = util::numToString(r.m_min);
            maxVal = util::numToString(r.m_max);
        }

        util::StringsList conds;
        if (0U < r.m_sinceVersion) {
            conds.push_back('(' + util::numToString(r.m_sinceVersion) + " <= Base::getVersion())");
        }

        if (r.m_deprecatedSince < commsdsl::parse::Protocol::notYetDeprecated()) {
            conds.push_back("(Base::getVersion() < " + util::numToString(r.m_deprecatedSince) + ")");
        }

        if (r.m_min == r.m_max) {
            conds.push_back("(static_cast<ValueType>(" + minVal + ") == Base::getValue())");
        }
        else {
            conds.push_back("(static_cast<ValueType>(" + minVal + ") <= Base::getValue())");
            conds.push_back("(Base::getValue() <= static_cast<ValueType>(" + maxVal + "))");
        }

        util::ReplacementMap rangeRepl = {
            {"COND", util::strListToString(conds, " &&\n", "")}
        };
        rangesChecks += util::processTemplate(RangeTempl, rangeRepl);
    }

    util::ReplacementMap repl = {
        {"RANGES_CHECKS", std::move(rangesChecks)},
    };

    return util::processTemplate(Templ, repl);    
}

std::size_t CommsIntField::commsMinLengthImpl() const
{
    if (intDslObj().availableLengthLimit()) {
        return 1U;
    }

    return CommsBase::commsMinLengthImpl();
}

std::string CommsIntField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
{
    static_cast<void>(accStr);
    assert(accStr.empty());
    try {
        if (isUnsignedType()) {
            return util::numToString(static_cast<std::uintmax_t>(std::stoull(value, nullptr, 0)));
        }

        return util::numToString(static_cast<std::intmax_t>(std::stoll(value, nullptr, 0)));
    }
    catch (...) {
        // nothing to do
    }

    static constexpr bool Not_yet_implemented = false;
    static_cast<void>(Not_yet_implemented);
    assert(Not_yet_implemented);
    return "???";
}

std::string CommsIntField::commsCommonHasSpecialsFuncCodeInternal() const
{
    auto& specials = specialsSortedByValue();    
    util::ReplacementMap repl = {
        {"VALUE", util::boolToString(!specials.empty())}
    };

    return util::processTemplate(hasSpecialsFuncTempl(), repl);
}

std::string CommsIntField::commsCommonValueNamesMapCodeInternal() const
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

std::string CommsIntField::commsCommonSpecialsCodeInternal() const
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
            "    return static_cast<ValueType>(#^#SPEC_VAL#$#);\n"
            "}\n"
        );

        std::string specVal;
        auto obj = intDslObj();
        auto type = obj.type();
        if ((type == commsdsl::parse::IntField::Type::Uint64) ||
            (type == commsdsl::parse::IntField::Type::Uintvar)) {
            specVal = util::numToString(static_cast<std::uintmax_t>(s.second.m_value));
        }
        else {
            specVal = util::numToString(s.second.m_value);
        }

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
            {"SPEC_VAL", std::move(specVal)},
            {"SPECIAL_DOC", std::move(desc)},
        };

        specialsList.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(specialsList, "\n", "\n");
}

std::string CommsIntField::commsCommonSpecialNamesMapCodeInternal() const
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

std::string CommsIntField::commsDefFieldOptsInternal(bool variantPropKey) const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddLengthOptInternal(opts);
    commsAddSerOffsetOptInternal(opts);
    commsAddScalingOptInternal(opts);
    commsAddUnitsOptInternal(opts);
    if (!variantPropKey) {
        commsAddDefaultValueOptInternal(opts);
        commsAddValidRangesOptInternal(opts);
        commsAddCustomRefreshOptInternal(opts);
        commsAddAvailableLengthLimitOptInternal(opts);
    }

    return util::strListToString(opts, ",\n", "");
}

std::string CommsIntField::commsDefValueNamesMapCodeInternal() const
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

std::string CommsIntField::commsDefHasSpecialsFuncCodeInternal() const
{
    util::ReplacementMap repl = {
        {"VALUE", comms::commonScopeFor(*this, generator()) + "::hasSpecials()"}
    };

    return util::processTemplate(hasSpecialsFuncTempl(), repl);
}

std::string CommsIntField::commsDefSpecialsCodeInternal() const
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
            "/// @see @ref #^#COMMON#$#::value#^#SPEC_ACC#$#().\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return #^#COMMON#$#::value#^#SPEC_ACC#$#();\n"
            "}\n\n"
            "/// @brief Check the value is equal to special @ref value#^#SPEC_ACC#$#().\n"
            "bool is#^#SPEC_ACC#$#() const\n"
            "{\n"
            "    return Base::getValue() == value#^#SPEC_ACC#$#();\n"
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
        };

        specialsList.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(specialsList, "\n", "");
}

std::string CommsIntField::commsDefSpecialNamesMapCodeInternal() const
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

std::string CommsIntField::commsDefDisplayDecimalsCodeInternal() const
{
    auto obj = intDslObj();
    auto scaling = obj.scaling();
    std::string result;
    if (scaling.first == scaling.second) {
        return result;
    }

    static const std::string Templ = 
        "/// @brief Requested number of digits after decimal point when value\n"
        "///     is displayed.\n"
        "static constexpr unsigned displayDecimals()\n"
        "{\n"
        "    return #^#DISPLAY_DECIMALS#$#;\n"
        "}";
        
    util::ReplacementMap repl = {
        {"DISPLAY_DECIMALS", util::numToString(obj.displayDecimals())}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsIntField::commsDefBaseClassInternal(bool variantPropKey) const
{
    static const std::string Templ = 
        "comms::field::IntValue<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    #^#FIELD_TYPE#$##^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = generator();
    auto dslObj = intDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.mainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(dslObj.endian())},
        {"FIELD_TYPE", comms::cppIntTypeFor(dslObj.type(), dslObj.maxLength())},
        {"FIELD_OPTS", commsDefFieldOptsInternal(variantPropKey)}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }
    return util::processTemplate(Templ, repl);      
}

void CommsIntField::commsAddLengthOptInternal(StringsList& opts) const
{
    auto obj = intDslObj();
    auto type = obj.type();
    if ((type == commsdsl::parse::IntField::Type::Intvar) ||
        (type == commsdsl::parse::IntField::Type::Uintvar)) {
        auto str =
            "comms::option::def::VarLength<" +
            util::numToString(obj.minLength()) +
            ", " +
            util::numToString(obj.maxLength()) +
            '>';
        util::addToStrList(std::move(str), opts);
        return;
    }

    if (obj.bitLength() != 0U) {
        auto str =
            "comms::option::def::FixedBitLength<" +
            util::numToString(obj.bitLength()) +
            '>';
        util::addToStrList(std::move(str), opts);
        return;
    }

    static const unsigned LengthMap[] = {
        /* Int8 */ 1,
        /* Uint8 */ 1,
        /* Int16 */ 2,
        /* Uint16 */ 2,
        /* Int32 */ 4,
        /* Uint32 */ 4,
        /* Int64 */ 8,
        /* Uint64 */ 8,
        /* Intvar */ 0,
        /* Uintvar */ 0
    };

    static const std::size_t LengthMapSize = std::extent<decltype(LengthMap)>::value;
    static_assert(LengthMapSize == static_cast<std::size_t>(commsdsl::parse::IntField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(type);
    if (LengthMapSize <= idx) {
        return;
    }

    assert(LengthMap[idx] != 0);
    if (LengthMap[idx] != obj.minLength()) {
        std::string secondParam;
        if (!obj.signExt()) {
            secondParam = ", false";
        }
        auto str =
            "comms::option::def::FixedLength<" +
            util::numToString(obj.minLength()) + secondParam +
            '>';
        util::addToStrList(std::move(str), opts);
    }
}

void CommsIntField::commsAddSerOffsetOptInternal(StringsList& opts) const
{
    auto obj = intDslObj();
    auto serOffset = obj.serOffset();
    if (serOffset == 0) {
        return;
    }

    auto str =
        "comms::option::def::NumValueSerOffset<" +
        util::numToString(serOffset) +
        '>';
    util::addToStrList(std::move(str), opts);
}

void CommsIntField::commsAddScalingOptInternal(StringsList& opts) const
{
    auto obj = intDslObj();
    auto scaling = obj.scaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return;
    }

    if ((num == 0) || (denom == 0)) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return;
    }

    auto str =
        "comms::option::def::ScalingRatio<" +
        util::numToString(num) +
        ", " +
        util::numToString(denom) +
        '>';
    util::addToStrList(std::move(str), opts);
}

void CommsIntField::commsAddUnitsOptInternal(StringsList& opts) const
{
    auto obj = intDslObj();
    auto units = obj.units();
    auto& str = comms::dslUnitsToOpt(units);
    if (!str.empty()) {
        util::addToStrList(str, opts);
    }
}

void CommsIntField::commsAddDefaultValueOptInternal(StringsList& opts) const
{
    auto obj = intDslObj();
    auto defaultValue = obj.defaultValue();
    if ((defaultValue == 0) &&
        (obj.semanticType() == commsdsl::parse::Field::SemanticType::Version)) {
        std::string str = "comms::option::def::DefaultNumValue<";
        str += util::numToString(generator().schemaVersion());
        str += '>';
        util::addToStrList(std::move(str), opts);
        return;
    }

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.type();
    if ((defaultValue < 0) &&
        ((type == commsdsl::parse::IntField::Type::Uint64) || (type == commsdsl::parse::IntField::Type::Uintvar))) {
        auto str =
            "comms::option::def::DefaultBigUnsignedNumValue<" +
            util::numToString(static_cast<std::uintmax_t>(defaultValue)) +
            '>';
        util::addToStrList(std::move(str), opts);
        return;
    }

    auto str =
        "comms::option::def::DefaultNumValue<" +
        util::numToString(defaultValue) +
        '>';
    util::addToStrList(std::move(str), opts);
}

void CommsIntField::commsAddValidRangesOptInternal(StringsList& opts) const
{
    auto obj = intDslObj();
    auto validRanges = obj.validRanges(); // copy
    if (validRanges.empty()) {
        return;
    }

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::parse::IntField::Type::Uint64) ||
        ((type != commsdsl::parse::IntField::Type::Uintvar) && (obj.maxLength() >= sizeof(std::int64_t)));

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        // unify
        std::size_t idx = 0U;
        while (idx < (validRanges.size() - 1U)) {
            auto& thisRange = validRanges[idx];
            auto& nextRange = validRanges[idx + 1];


            auto needToMergeCheck =
                [](auto min1, auto max1, auto min2, auto max2) -> bool
                {
                    static_cast<void>(min1);
                    static_cast<void>(max2);
                    assert(min1 <= min2);
                    if (min2 <= (max1 + 1)) {
                        assert(max1 <= max2);
                        return true;
                    }

                    return false;
                };

            bool merge = false;
            if (bigUnsigned) {
                merge =
                    needToMergeCheck(
                        static_cast<std::uintmax_t>(thisRange.m_min),
                        static_cast<std::uintmax_t>(thisRange.m_max),
                        static_cast<std::uintmax_t>(nextRange.m_min),
                        static_cast<std::uintmax_t>(nextRange.m_max));
            }
            else {
                merge = needToMergeCheck(thisRange.m_min, thisRange.m_max, nextRange.m_min, nextRange.m_max);
            }

            if (!merge) {
                ++idx;
                continue;
            }

            auto needToUpdateCheck =
                [](auto max1, auto max2) -> bool
                {
                    return max1 < max2;
                };

            bool update = false;
            if (bigUnsigned) {
                update =
                    needToUpdateCheck(
                        static_cast<std::uintmax_t>(thisRange.m_max),
                        static_cast<std::uintmax_t>(nextRange.m_max));
            }
            else {
                update = needToUpdateCheck(thisRange.m_max, nextRange.m_max);
            }

            if (update) {
                thisRange.m_max = nextRange.m_max;
            }

            validRanges.erase(validRanges.begin() + idx + 1);
        }
    }

    bool versionStorageRequired = false;
    bool addedRangeOpt = false;
    for (auto& r : validRanges) {
        if (!generator().doesElementExist(r.m_sinceVersion, r.m_deprecatedSince, !validCheckVersion)) {
            continue;
        }

        if (validCheckVersion && (generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince, false))) {
            versionStorageRequired = true;
            continue;
        }

        bool big = false;
        std::string str = "comms::option::def::";
        do {
            if (!bigUnsigned) {
                break;
            }

            bool minInRange =
                static_cast<std::uintmax_t>(r.m_min) <= static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());

            bool maxInRange =
                static_cast<std::uintmax_t>(r.m_max) <= static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());
            if (minInRange && maxInRange) {
                break;
            }

            if (r.m_min == r.m_max) {
                str += "ValidBigUnsignedNumValue<";
                str += util::numToString(static_cast<std::uintmax_t>(r.m_min));
                str += '>';
            }
            else {
                str += "ValidBigUnsignedNumValueRange<";
                str += util::numToString(static_cast<std::uintmax_t>(r.m_min));
                str += ", ";
                str += util::numToString(static_cast<std::uintmax_t>(r.m_max));
                str += '>';
            }

            util::addToStrList(std::move(str), opts);
            big = true;
            addedRangeOpt = true;
        } while (false);

        if (big) {
            continue;
        }

        if (r.m_min == r.m_max) {
            str += "ValidNumValue<";
            str += util::numToString(r.m_min);
            str += '>';
        }
        else {
            str += "ValidNumValueRange<";
            str += util::numToString(r.m_min);
            str += ", ";
            str += util::numToString(r.m_max);
            str += '>';
        }

        util::addToStrList(std::move(str), opts);
        addedRangeOpt = true;
    }

    if (versionStorageRequired) {
        util::addToStrList("comms::option::def::VersionStorage", opts);

        if (!addedRangeOpt) {
            util::addToStrList("comms::option::def::InvalidByDefault", opts);
        }
    }
}

void CommsIntField::commsAddCustomRefreshOptInternal(StringsList& opts) const
{
    if (commsRequiresFailOnInvalidRefreshInternal()) {
        util::addToStrList("comms::option::def::HasCustomRefresh", opts);
    }
}

void CommsIntField::commsAddAvailableLengthLimitOptInternal(StringsList& opts) const
{
    if (intDslObj().availableLengthLimit()) {
        util::addToStrList("comms::option::def::AvailableLengthLimit", opts);
    }
}

bool CommsIntField::commsRequiresFailOnInvalidRefreshInternal() const
{
    if (!dslObj().isFailOnInvalid()) {
        return false;
    }

    auto obj = intDslObj();
    auto& validRanges = obj.validRanges();
    if (validRanges.empty()) {
        return false;
    }

    return true;
}



} // namespace commsdsl2comms
