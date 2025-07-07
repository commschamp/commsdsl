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
    commsdsl::parse::ParseField dslObj, 
    commsdsl::gen::GenElem* parent) :
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
    auto obj = genIntFieldParseObj();
    if (!genIsUnsignedType()) {
        return util::genNumToString(obj.parseDefaultValue());
    }

    unsigned hexWidth = static_cast<unsigned>(obj.parseMaxLength() * 2U);
    auto val = static_cast<std::uintmax_t>(obj.parseDefaultValue());
    auto decValue = util::genNumToString(val);
    auto hexValue = util::genNumToString(val, hexWidth);

    static const std::string Templ = 
        "#^#DEC#$# /* #^#HEX#$# */";

    util::ReplacementMap repl = {
        {"DEC", std::move(decValue)},
        {"HEX", std::move(hexValue)},
    };
    return util::genProcessTemplate(Templ, repl);
}

bool CommsIntField::commsVariantIsValidPropKey() const
{
    auto obj = genIntFieldParseObj();
    if (!obj.parseIsFailOnInvalid()) {
        return false;
    }

    if (obj.parseIsPseudo()) {
        return false;
    }

    auto& validRanges = obj.parseValidRanges();
    if (validRanges.size() != 1U) {
        return false;
    }

    auto& r = validRanges.front();
    if (r.m_min != r.m_max) {
        return false;
    }

    if (r.m_min != obj.parseDefaultValue()) {
        return false;
    }

    return true;
}

bool CommsIntField::commsVariantIsPropKeyEquivalent(const CommsIntField& other) const
{
    auto thisDslObj = genIntFieldParseObj();
    auto otherDslObj = other.genIntFieldParseObj();

    auto thisType = comms::genCppIntTypeFor(thisDslObj.parseType(), thisDslObj.parseMaxLength());
    auto otherType = comms::genCppIntTypeFor(otherDslObj.parseType(), otherDslObj.parseMaxLength());
    if (thisType != otherType) {
        return false;
    }

    auto thisOpts = commsDefFieldOptsInternal(true);
    auto otherOpts = other.commsDefFieldOptsInternal(true);
    if (thisOpts != otherOpts) {
        return false;
    }

    return thisDslObj.parseEndian() == otherDslObj.parseEndian();
}

bool CommsIntField::genPrepareImpl()
{
    return Base::genPrepareImpl() && commsPrepare();
}

bool CommsIntField::genWriteImpl() const
{
    return commsWrite();
}

CommsIntField::IncludesList CommsIntField::commsCommonIncludesImpl() const
{
    IncludesList list = {
        "<cstdint>"
    };

    auto& specials = genSpecialsSortedByValue(); 
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

    //auto& specials = genSpecialsSortedByValue();

    auto& gen = genGenerator();
    auto dslObj = genIntFieldParseObj();
    util::ReplacementMap repl = {
        {"SCOPE", comms::genScopeFor(*this, gen, true, true)},
        {"VALUE_TYPE", comms::genCppIntTypeFor(dslObj.parseType(), dslObj.parseMaxLength())},
        {"SPECIAL_VALUE_NAMES_MAP_DEFS", commsCommonValueNamesMapCodeInternal()},
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"HAS_SPECIAL_FUNC", commsCommonHasSpecialsFuncCodeInternal()},
        {"SPECIALS", commsCommonSpecialsCodeInternal()},
        {"SPECIAL_NAMES_MAP", commsCommonSpecialNamesMapCodeInternal()},
    };
    return util::genProcessTemplate(Templ, repl);
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
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsIntField::commsDefRefreshFuncBodyImpl() const
{
    if (!commsRequiresFailOnInvalidRefreshInternal()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "bool updated = Base::refresh();\n"
        "if (Base::valid()) {\n"
        "    return updated;\n"
        "};\n"
        "Base::setValue(#^#VALID_VALUE#$#);\n"
        "return true;\n";

    auto obj = genIntFieldParseObj();
    auto& validRanges = obj.parseValidRanges();    
    util::ReplacementMap repl = {
        {"VALID_VALUE", util::genNumToString(validRanges.front().m_min)},
    };
    return util::genProcessTemplate(Templ, repl);    
}

std::string CommsIntField::commsDefValidFuncBodyImpl() const
{
    auto obj = genIntFieldParseObj();

    bool validCheckVersion =
        genGenerator().genSchemaOf(*this).genVersionDependentCode() &&
        obj.parseValidCheckVersion();

    if (!validCheckVersion) {
        return strings::genEmptyString();
    }

    auto validRanges = obj.parseValidRanges(); // copy
    validRanges.erase(
        std::remove_if(
            validRanges.begin(), validRanges.end(),
            [this](auto& r)
            {
                return !this->genGenerator().genIsElementOptional(r.m_sinceVersion, r.m_deprecatedSince, true);
            }),
        validRanges.end());

    if (validRanges.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "if (Base::valid()) {\n"
        "    return true;\n"
        "}\n\n"
        "#^#RANGES_CHECKS#$#\n"
        "return false;\n"
        ;

    auto type = obj.parseType();
    bool bigUnsigned =
        (type == commsdsl::parse::ParseIntField::ParseType::Uint64) ||
        (type == commsdsl::parse::ParseIntField::ParseType::Uintvar);

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
            minVal = util::genNumToString(static_cast<std::uintmax_t>(r.m_min));
            maxVal = util::genNumToString(static_cast<std::uintmax_t>(r.m_max));
        }
        else {
            minVal = util::genNumToString(r.m_min);
            maxVal = util::genNumToString(r.m_max);
        }

        util::GenStringsList conds;
        if (0U < r.m_sinceVersion) {
            conds.push_back('(' + util::genNumToString(r.m_sinceVersion) + " <= Base::getVersion())");
        }

        if (r.m_deprecatedSince < commsdsl::parse::ParseProtocol::parseNotYetDeprecated()) {
            conds.push_back("(Base::getVersion() < " + util::genNumToString(r.m_deprecatedSince) + ")");
        }

        if (r.m_min == r.m_max) {
            conds.push_back("(static_cast<ValueType>(" + minVal + ") == Base::getValue())");
        }
        else {
            conds.push_back("(static_cast<ValueType>(" + minVal + ") <= Base::getValue())");
            conds.push_back("(Base::getValue() <= static_cast<ValueType>(" + maxVal + "))");
        }

        util::ReplacementMap rangeRepl = {
            {"COND", util::genStrListToString(conds, " &&\n", "")}
        };
        rangesChecks += util::genProcessTemplate(RangeTempl, rangeRepl);
    }

    util::ReplacementMap repl = {
        {"RANGES_CHECKS", std::move(rangesChecks)},
    };

    return util::genProcessTemplate(Templ, repl);    
}

bool CommsIntField::commsIsVersionDependentImpl() const
{
    assert(genGenerator().genSchemaOf(*this).genVersionDependentCode());
    auto obj = genIntFieldParseObj();
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

std::size_t CommsIntField::commsMinLengthImpl() const
{
    if (genIntFieldParseObj().parseAvailableLengthLimit()) {
        return 1U;
    }

    return CommsBase::commsMinLengthImpl();
}

std::string CommsIntField::commsCompPrepValueStrImpl([[maybe_unused]] const std::string& accStr, const std::string& value) const
{
    assert(accStr.empty());

    auto valToString = 
        [this](std::intmax_t val)
        {
            if (genIsUnsignedType()) {
                return util::genNumToString(static_cast<std::uintmax_t>(val));
            }

            return util::genNumToString(val);      
        };

    if (value.empty()) {
        return valToString(genIntFieldParseObj().parseDefaultValue());
    }
    
    try {
        if (genIsUnsignedType()) {
            return util::genNumToString(static_cast<std::uintmax_t>(std::stoull(value, nullptr, 0)));
        }

        return util::genNumToString(static_cast<std::intmax_t>(std::stoll(value, nullptr, 0)));
    }
    catch (...) {
        // nothing to do
    }

    auto& specials = genIntFieldParseObj().parseSpecialValues();
    auto iter = specials.find(value);
    if (iter != specials.end()) {
        return valToString(iter->second.m_value);
    }

    auto& gen = genGenerator();
    do {
        auto pos = value.find_first_of(".");
        auto fieldRef = value; // copy
        std::string valueSubstr;

        if (pos < value.size() && (value[0] == strings::genSchemaRefPrefix())) {
            pos = value.find_first_of(".", pos + 1); // find second
        }

        if (pos < value.size()) {
            fieldRef = value.substr(0, pos);
            valueSubstr = value.substr(pos + 1);
        }

        auto field = gen.genFindField(fieldRef);
        if (field == nullptr) {
            break;        
        }

        auto* commsField = dynamic_cast<const CommsField*>(field);
        assert(commsField != nullptr);
        auto newValue = commsField->commsCompPrepValueStr(std::string(), valueSubstr);
        if (newValue.empty() || (newValue == strings::genUnexpectedValueStr())) {
            break;
        }

        return commsCompPrepValueStrImpl(std::string(), newValue);
    } while (false);

    gen.genLogger().genError("Unknown value comparison string \"" + value + "\" for field " + comms::genScopeFor(*this, gen));
    [[maybe_unused]] static constexpr bool Not_yet_implemented = false;
    assert(Not_yet_implemented);
    return strings::genUnexpectedValueStr();
}

bool CommsIntField::commsVerifyInnerRefImpl(const std::string& refStr) const
{
    auto obj = genIntFieldParseObj();
    auto& specials = obj.parseSpecialValues();
    return (specials.find(refStr) != specials.end());    
}

std::string CommsIntField::commsCommonHasSpecialsFuncCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();    
    util::ReplacementMap repl = {
        {"VALUE", util::genBoolToString(!specials.empty())}
    };

    return util::genProcessTemplate(hasSpecialsFuncTempl(), repl);
}

std::string CommsIntField::commsCommonValueNamesMapCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();    
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::ReplacementMap repl = {
        {"INFO_DEF", "std::pair<ValueType, const char*>"},
        {"MAP_DEF", "std::pair<const SpecialNameInfo*, std::size_t>"}
    };

    return util::genProcessTemplate(specialNamesMapTempl(), repl);
}

std::string CommsIntField::commsCommonSpecialsCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    for (auto& s : specials) {
        if (!genGenerator().genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
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
        auto obj = genIntFieldParseObj();
        auto type = obj.parseType();
        if ((type == commsdsl::parse::ParseIntField::ParseType::Uint64) ||
            (type == commsdsl::parse::ParseIntField::ParseType::Uintvar)) {
            specVal = util::genNumToString(static_cast<std::uintmax_t>(s.second.m_value));
        }
        else {
            specVal = util::genNumToString(s.second.m_value);
        }

        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = util::genStrMakeMultiline(desc);
            desc = util::genStrReplace(desc, "\n", "\n///     ");
        }

        util::ReplacementMap repl = {
            {"SPEC_NAME", s.first},
            {"SPEC_ACC", comms::genClassName(s.first)},
            {"SPEC_VAL", std::move(specVal)},
            {"SPECIAL_DOC", std::move(desc)},
        };

        specialsList.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(specialsList, "\n", "\n");
}

std::string CommsIntField::commsCommonSpecialNamesMapCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
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

    util::GenStringsList specialInfos;
    for (auto& s : specials) {
        static const std::string SpecTempl = 
            "std::make_pair(value#^#SPEC_ACC#$#(), \"#^#SPEC_NAME#$#\")";

        util::ReplacementMap specRepl = {
            {"SPEC_ACC", comms::genClassName(s.first)},
            {"SPEC_NAME", s.first}
        };
        specialInfos.push_back(util::genProcessTemplate(SpecTempl, specRepl));
    }

    util::ReplacementMap repl {
        {"INFOS", util::genStrListToString(specialInfos, ",\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsIntField::commsDefFieldOptsInternal(bool variantPropKey) const
{
    util::GenStringsList opts;

    commsAddFieldDefOptions(opts, variantPropKey);
    commsAddLengthOptInternal(opts);
    commsAddSerOffsetOptInternal(opts);
    commsAddDisplayOffsetOptInternal(opts);
    commsAddScalingOptInternal(opts);
    commsAddUnitsOptInternal(opts);
    if (!variantPropKey) {
        commsAddDefaultValueOptInternal(opts);
        commsAddValidRangesOptInternal(opts);
        commsAddCustomRefreshOptInternal(opts);
        commsAddAvailableLengthLimitOptInternal(opts);
    }

    return util::genStrListToString(opts, ",\n", "");
}

std::string CommsIntField::commsDefValueNamesMapCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();    
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    auto scope = comms::genCommonScopeFor(*this, genGenerator());

    util::ReplacementMap repl = {
        {"INFO_DEF", scope + "::SpecialNameInfo"},
        {"MAP_DEF", scope + "::SpecialNamesMapInfo"}
    };

    return util::genProcessTemplate(specialNamesMapTempl(), repl);    
}

std::string CommsIntField::commsDefHasSpecialsFuncCodeInternal() const
{
    util::ReplacementMap repl = {
        {"VALUE", comms::genCommonScopeFor(*this, genGenerator()) + "::hasSpecials()"}
    };

    return util::genProcessTemplate(hasSpecialsFuncTempl(), repl);
}

std::string CommsIntField::commsDefSpecialsCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    for (auto& s : specials) {
        if (!genGenerator().genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
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
            {"SPEC_ACC", comms::genClassName(s.first)},
            {"COMMON", comms::genCommonScopeFor(*this, genGenerator())},
        };

        specialsList.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(specialsList, "\n", "");
}

std::string CommsIntField::commsDefSpecialNamesMapCodeInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "/// @brief Retrieve map of special value names\n"
        "static SpecialNamesMapInfo specialNamesMap()\n"
        "{\n"
        "    return #^#COMMON#$#::specialNamesMap();\n"
        "}\n";    

    util::ReplacementMap repl {
        {"COMMON", comms::genCommonScopeFor(*this, genGenerator())}
    };

    return util::genProcessTemplate(Templ, repl);    
}

std::string CommsIntField::commsDefDisplayDecimalsCodeInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
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
        {"DISPLAY_DECIMALS", util::genNumToString(obj.parseDisplayDecimals())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsIntField::commsDefBaseClassInternal(bool variantPropKey) const
{
    static const std::string Templ = 
        "comms::field::IntValue<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    #^#FIELD_TYPE#$##^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";  

    auto& gen = genGenerator();
    auto dslObj = genIntFieldParseObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.genSchemaOf(*this).genMainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(dslObj.parseEndian())},
        {"FIELD_TYPE", comms::genCppIntTypeFor(dslObj.parseType(), dslObj.parseMaxLength())},
        {"FIELD_OPTS", commsDefFieldOptsInternal(variantPropKey)}
    };

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }
    return util::genProcessTemplate(Templ, repl);      
}

void CommsIntField::commsAddLengthOptInternal(StringsList& opts) const
{
    auto obj = genIntFieldParseObj();
    auto type = obj.parseType();
    if ((type == commsdsl::parse::ParseIntField::ParseType::Intvar) ||
        (type == commsdsl::parse::ParseIntField::ParseType::Uintvar)) {
        auto str =
            "comms::option::def::VarLength<" +
            util::genNumToString(obj.parseMinLength()) +
            ", " +
            util::genNumToString(obj.parseMaxLength()) +
            '>';
        util::genAddToStrList(std::move(str), opts);
        return;
    }

    if (obj.parseBitLength() != 0U) {
        std::string secondParam;
        if (!obj.parseSignExt()) {
            secondParam = ", false";
        }

        auto str =
            "comms::option::def::FixedBitLength<" +
            util::genNumToString(obj.parseBitLength()) + secondParam +
            '>';
        util::genAddToStrList(std::move(str), opts);
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
    static_assert(LengthMapSize == static_cast<std::size_t>(commsdsl::parse::ParseIntField::ParseType::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(type);
    if (LengthMapSize <= idx) {
        return;
    }

    assert(LengthMap[idx] != 0);
    if (LengthMap[idx] != obj.parseMinLength()) {
        std::string secondParam;
        if (!obj.parseSignExt()) {
            secondParam = ", false";
        }
        auto str =
            "comms::option::def::FixedLength<" +
            util::genNumToString(obj.parseMinLength()) + secondParam +
            '>';
        util::genAddToStrList(std::move(str), opts);
    }
}

void CommsIntField::commsAddSerOffsetOptInternal(StringsList& opts) const
{
    auto obj = genIntFieldParseObj();
    auto serOffset = obj.parseSerOffset();
    if (serOffset == 0) {
        return;
    }

    auto str =
        "comms::option::def::NumValueSerOffset<" +
        util::genNumToString(serOffset) +
        '>';
    util::genAddToStrList(std::move(str), opts);
}

void CommsIntField::commsAddDisplayOffsetOptInternal(StringsList& opts) const
{
    auto obj = genIntFieldParseObj();
    auto displayOffset = obj.parseDisplayOffset();
    if (displayOffset == 0) {
        return;
    }

    auto str =
        "comms::option::def::DisplayOffset<" +
        util::genNumToString(displayOffset) +
        '>';
    util::genAddToStrList(std::move(str), opts);
}

void CommsIntField::commsAddScalingOptInternal(StringsList& opts) const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return;
    }

    if ((num == 0) || (denom == 0)) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return;
    }

    auto str =
        "comms::option::def::ScalingRatio<" +
        util::genNumToString(num) +
        ", " +
        util::genNumToString(denom) +
        '>';
    util::genAddToStrList(std::move(str), opts);
}

void CommsIntField::commsAddUnitsOptInternal(StringsList& opts) const
{
    auto obj = genIntFieldParseObj();
    auto units = obj.parseUnits();
    auto& str = comms::genParseUnitsToOpt(units);
    if (!str.empty()) {
        util::genAddToStrList(str, opts);
    }
}

void CommsIntField::commsAddDefaultValueOptInternal(StringsList& opts) const
{
    auto obj = genIntFieldParseObj();
    auto defaultValue = obj.parseDefaultValue();
    if ((defaultValue == 0) &&
        (obj.parseSemanticType() == commsdsl::parse::ParseField::ParseSemanticType::Version)) {
        std::string str = "comms::option::def::DefaultNumValue<";
        str += util::genNumToString(genGenerator().genSchemaOf(*this).genSchemaVersion());
        str += '>';
        util::genAddToStrList(std::move(str), opts);
        return;
    }

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.parseType();
    if ((defaultValue < 0) &&
        ((type == commsdsl::parse::ParseIntField::ParseType::Uint64) || (type == commsdsl::parse::ParseIntField::ParseType::Uintvar))) {
        auto str =
            "comms::option::def::DefaultBigUnsignedNumValue<" +
            util::genNumToString(static_cast<std::uintmax_t>(defaultValue)) +
            '>';
        util::genAddToStrList(std::move(str), opts);
        return;
    }

    auto str =
        "comms::option::def::DefaultNumValue<" +
        util::genNumToString(defaultValue) +
        '>';
    util::genAddToStrList(std::move(str), opts);
}

void CommsIntField::commsAddValidRangesOptInternal(StringsList& opts) const
{
    auto obj = genIntFieldParseObj();
    auto validRanges = obj.parseValidRanges(); // copy
    if (validRanges.empty()) {
        return;
    }

    auto type = obj.parseType();
    bool bigUnsigned =
        (type == commsdsl::parse::ParseIntField::ParseType::Uint64) ||
        ((type != commsdsl::parse::ParseIntField::ParseType::Uintvar) && (obj.parseMaxLength() >= sizeof(std::int64_t)));

    bool validCheckVersion =
        genGenerator().genSchemaOf(*this).genVersionDependentCode() &&
        obj.parseValidCheckVersion();

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
        if (!genGenerator().genDoesElementExist(r.m_sinceVersion, r.m_deprecatedSince, !validCheckVersion)) {
            continue;
        }

        if (validCheckVersion && (genGenerator().genIsElementOptional(r.m_sinceVersion, r.m_deprecatedSince, false))) {
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
                str += util::genNumToString(static_cast<std::uintmax_t>(r.m_min));
                str += '>';
            }
            else {
                str += "ValidBigUnsignedNumValueRange<";
                str += util::genNumToString(static_cast<std::uintmax_t>(r.m_min));
                str += ", ";
                str += util::genNumToString(static_cast<std::uintmax_t>(r.m_max));
                str += '>';
            }

            util::genAddToStrList(std::move(str), opts);
            big = true;
            addedRangeOpt = true;
        } while (false);

        if (big) {
            continue;
        }

        if (r.m_min == r.m_max) {
            str += "ValidNumValue<";
            str += util::genNumToString(r.m_min);
            str += '>';
        }
        else {
            str += "ValidNumValueRange<";
            str += util::genNumToString(r.m_min);
            str += ", ";
            str += util::genNumToString(r.m_max);
            str += '>';
        }

        util::genAddToStrList(std::move(str), opts);
        addedRangeOpt = true;
    }

    if (versionStorageRequired) {
        util::genAddToStrList("comms::option::def::VersionStorage", opts);

        if (!addedRangeOpt) {
            util::genAddToStrList("comms::option::def::InvalidByDefault", opts);
        }
    }
}

void CommsIntField::commsAddCustomRefreshOptInternal(StringsList& opts) const
{
    if (commsRequiresFailOnInvalidRefreshInternal()) {
        util::genAddToStrList("comms::option::def::HasCustomRefresh", opts);
    }
}

void CommsIntField::commsAddAvailableLengthLimitOptInternal(StringsList& opts) const
{
    if (genIntFieldParseObj().parseAvailableLengthLimit()) {
        util::genAddToStrList("comms::option::def::AvailableLengthLimit", opts);
    }
}

bool CommsIntField::commsRequiresFailOnInvalidRefreshInternal() const
{
    if ((!genParseObj().parseIsFailOnInvalid()) ||
        (genParseObj().parseIsFixedValue())) {
        return false;
    }

    auto obj = genIntFieldParseObj();
    auto& validRanges = obj.parseValidRanges();
    if (validRanges.empty()) {
        return false;
    }

    return true;
}



} // namespace commsdsl2comms
