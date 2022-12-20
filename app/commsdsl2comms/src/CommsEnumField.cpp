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

#include "CommsEnumField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

namespace 
{

const std::size_t MaxRangesInOpts = 5U;

} // namespace 
    

CommsEnumField::CommsEnumField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

commsdsl::gen::util::StringsList CommsEnumField::commsEnumValues() const
{
    commsdsl::gen::util::StringsList result;
    if (m_validRanges.empty()) {
        return result;
    }
    
    auto& revValues = sortedRevValues();
    util::StringsList valuesStrings;
    valuesStrings.reserve(revValues.size() + 3);
    auto obj = enumDslObj();
    auto& values = obj.values();

    for (auto& v : revValues) {
        auto iter = values.find(*v.second);
        if (iter == values.end()) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            continue;
        }

        bool exists =
            generator().doesElementExist(
                iter->second.m_sinceVersion,
                iter->second.m_deprecatedSince,
                false);

        if (!exists) {
            continue;
        }

        static const std::string Templ =
            "#^#NAME#$# = #^#VALUE#$#, ";


        std::string valStr = valueToString(v.first);
        std::string docStr;
        if (!iter->second.m_description.empty()) {
            docStr = " ///< " + iter->second.m_description;
            docStr = util::strMakeMultiline(docStr, 40);
        }
        else if (dslObj().semanticType() == commsdsl::parse::Field::SemanticType::MessageId) {
            if (!iter->second.m_displayName.empty()) {
                docStr = " ///< message id of <b>" + iter->second.m_displayName + "</b> message.";
            }
            else {
                docStr = " ///< message id of @b " + *v.second + " message.";
            }
        }
        else if (!iter->second.m_displayName.empty()) {
            docStr = " ///< value <b>" + iter->second.m_displayName + "</b>.";
        }
        else {
            docStr = " ///< value @b " + *v.second + '.';
        }

        auto deprecatedVer = iter->second.m_deprecatedSince;
        if (generator().isElementDeprecated(deprecatedVer)) {
            docStr += "\nDeprecated since version " + std::to_string(deprecatedVer) + '.';
        }
        docStr = util::strReplace(docStr, "\n", "\n///  ");

        assert(!valStr.empty());
        util::ReplacementMap repl = {
            {"NAME", *v.second},
            {"VALUE", std::move(valStr)},
        };
        auto templ = util::processTemplate(Templ, repl);
        assert(2U <= templ.size());
        static const std::string DocElem("#^#DOC#$#");
        templ.insert((templ.end() - 1U), DocElem.begin(), DocElem.end());

        util::ReplacementMap docRepl = {
            {"DOC", std::move(docStr)}
        };
        valuesStrings.push_back(util::processTemplate(templ, docRepl));
    }

    if (!revValues.empty()) {
        auto addNameSuffixFunc =
            [&values](const std::string& n) -> std::string
            {
                std::string suffix;
                while (true) {
                    auto s = n + suffix;
                    if (values.find(s) == values.end()) {
                        return s;
                    }

                    suffix += '_';
                }
            };

        auto& firstElem = revValues.front();
        assert(firstElem.second != nullptr);
        assert(!firstElem.second->empty());
        auto firstLetter = firstElem.second->front();
        bool useLower = (std::tolower(firstLetter) == static_cast<int>(firstLetter));

        auto adjustFirstLetterNameFunc =
            [useLower](const std::string& s)
            {
                if (!useLower) {
                    assert(!s.empty());
                    assert(s[0] == static_cast<char>(std::toupper(s[0])));
                    return s;
                }

                auto sCpy = s;
                assert(!s.empty());
                sCpy[0] = static_cast<char>(std::tolower(sCpy[0]));
                return sCpy;
            };

        auto createValueStrFunc =
            [this, &adjustFirstLetterNameFunc, &addNameSuffixFunc](const std::string& n, std::intmax_t val, const std::string& doc) -> std::string
            {
                return
                    adjustFirstLetterNameFunc(addNameSuffixFunc(n)) + " = " +
                    valueToString(val) + ", ///< " + doc;

            };

        valuesStrings.push_back("\n// --- Extra values generated for convenience ---");
        valuesStrings.push_back(createValueStrFunc(strings::enumFirstValueStr(), revValues.front().first, "First defined value."));
        valuesStrings.push_back(createValueStrFunc(strings::enumLastValueStr(), revValues.back().first, "Last defined value."));

        if (hasValuesLimit()) {
            valuesStrings.push_back(createValueStrFunc(strings::enumValuesLimitStr(), revValues.back().first + 1, "Upper limit for defined values."));
        }
    }

    return valuesStrings;    
}

bool CommsEnumField::prepareImpl()
{
    return 
        Base::prepareImpl() && 
        commsPrepare() &&
        commsPrepareValidRangesInternal();
}

bool CommsEnumField::writeImpl() const
{
    return commsWrite();
}

CommsEnumField::IncludesList CommsEnumField::commsCommonIncludesImpl() const
{
    IncludesList result = {
        "<cstdint>",
        "<type_traits>",
        "<utility>"
    };

    if (enumDslObj().semanticType() == commsdsl::parse::Field::SemanticType::MessageId) {
        auto inc = comms::relHeaderForRoot(strings::msgIdEnumNameStr(), generator());
        result.push_back(std::move(inc));
    }  

    if ((MaxRangesInOpts < m_validRanges.size()) ||
        (!commsIsDirectValueNameMappingInternal())) {
        result.insert(result.end(), {
            "<algorithm>",
            "<iterator>"
        });
    }

    return result;  
}

std::string CommsEnumField::commsCommonCodeBodyImpl() const
{
    static const std::string Templ = 
        "#^#ENUM_DEF#$#\n"
        "#^#VALUE_NAME_MAP_DEF#$#\n"
        "#^#NAME_FUNC#$#\n"
        "#^#VAL_NAME_FUNC#$#\n"
        "#^#VAL_VALUE_NAMES_MAP_FUNC#$#\n"
    ;

    util::ReplacementMap repl = {
        {"ENUM_DEF", commsCommonEnumInternal()},
        {"VALUE_NAME_MAP_DEF", commsCommonValueNameMapInternal()},
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"VAL_NAME_FUNC", commsCommonValueNameFuncCodeInternal()},
        {"VAL_VALUE_NAMES_MAP_FUNC", commsCommonValueNamesMapFuncCodeInternal()}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsCommonCodeExtraImpl() const
{
    static const std::string Templ = 
        "\n"
        "/// @brief Values enumerator for\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "using #^#NAME#$#Val = #^#NAME#$#Common::ValueType;\n";

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, generator())},
        {"NAME", comms::className(dslObj().name())}
    };
    return util::processTemplate(Templ, repl);
}

CommsEnumField::IncludesList CommsEnumField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/EnumValue.h"
    };

    return result;
}

std::string CommsEnumField::commsDefExtraDoxigenImpl() const
{
    if (dslObj().semanticType() == commsdsl::parse::Field::SemanticType::MessageId) {
        return "@see @ref " + comms::scopeForRoot(strings::msgIdEnumNameStr(), generator());
    }   

    return "@see @ref " + comms::commonScopeFor(*this, generator()) + "::ValueType"; 
}

std::string CommsEnumField::commsDefBaseClassImpl() const
{
    static const std::string Templ = 
        "comms::field::EnumValue<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    #^#COMMON_SCOPE#$#::ValueType#^#COMMA#$#\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";

    auto& gen = generator();
    auto dslObj = enumDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.schemaOf(*this).mainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(dslObj.endian())},
        {"COMMON_SCOPE", comms::commonScopeFor(*this, gen)},
        {"FIELD_OPTS", commsDefFieldOptsInternal()}
    };         

    if (!repl["FIELD_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsDefPublicCodeImpl() const
{
    static const std::string Templ = 
        "/// @brief Re-definition of the value type.\n"
        "using ValueType = typename Base::ValueType;\n\n"
        "#^#VALUE_NAMES_MAP_DEFS#$#\n"
        "#^#VALUE_NAME#$#\n"
        "#^#VALUE_NAMES_MAP#$#\n"    
    ;

    util::ReplacementMap repl = {
        {"VALUE_NAMES_MAP_DEFS", commsDefValueNameMapInternal()},
        {"VALUE_NAME", commsDefValueNameFuncCodeInternal()},
        {"VALUE_NAMES_MAP", commsDefValueNamesMapFuncCodeInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsDefValidFuncBodyImpl() const
{
    auto obj = enumDslObj();
    bool validCheckVersion =
        generator().schemaOf(*this).versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {

        if (m_validRanges.size() <= MaxRangesInOpts) {
            return strings::emptyString(); // Already in options
        }

        static const std::string Templ =
            "if (!Base::valid()) {\n"
            "    return false;\n"
            "}\n\n"
            "static const ValueType Values[] = {\n"
            "    #^#VALUES#$#\n"
            "};\n\n"
            "auto iter =\n"
            "    std::lower_bound(std::begin(Values), std::end(Values), Base::getValue());\n\n"
            "if ((iter == std::end(Values)) || (*iter != Base::getValue())) {\n"
            "    return false;\n"
            "}\n\n"
            "return true;\n"
            ;

        util::StringsList valuesStrings;

        bool isMessageId =
            obj.semanticType() == commsdsl::parse::Field::SemanticType::MessageId;
        auto& revValues = obj.revValues();
        auto prevIter = revValues.end();
        for (auto iter = revValues.begin(); iter != revValues.end(); ++iter) {

            if ((prevIter != revValues.end()) && (prevIter->first == iter->first)) {
                continue;
            }

            std::string prefix;
            if (isMessageId) {
                 prefix = generator().schemaOf(*this).mainNamespace() + "::" + strings::msgIdPrefixStr();
            }
            else {
                prefix = "ValueType::";
            }

            valuesStrings.push_back(prefix + iter->second);
            prevIter = iter;
        }

        util::ReplacementMap repl = {
            {"VALUES", util::strListToString(valuesStrings, ",\n", "")}
        };

        return util::processTemplate(Templ, repl);
    }

    // version must be taken into account
    std::vector<decltype(m_validRanges)> rangesToProcess;
    for (auto& r : m_validRanges) {
        if ((r.m_sinceVersion == 0U) &&
            (r.m_deprecatedSince == commsdsl::parse::Protocol::notYetDeprecated())) {
            continue;
        }

        if ((rangesToProcess.empty()) ||
            (rangesToProcess.back().back().m_sinceVersion != r.m_sinceVersion) ||
            (rangesToProcess.back().back().m_deprecatedSince != r.m_deprecatedSince)){
            rangesToProcess.resize(rangesToProcess.size() + 1);
        }

        rangesToProcess.back().push_back(r);
    }

    static const std::string VersionBothCondTempl =
        "if ((#^#FROM_VERSION#$# <= Base::getVersion()) &&\n"
        "    (Base::getVersion() < #^#UNTIL_VERSION#$#)) {\n"
        "    #^#COMPARISONS#$#\n"
        "}\n";

    static const std::string VersionFromCondTempl =
        "if (#^#FROM_VERSION#$# <= Base::getVersion()) {\n"
        "    #^#COMPARISONS#$#\n"
        "}\n";

    static const std::string VersionUntilCondTempl =
        "if (Base::getVersion() < #^#UNTIL_VERSION#$#) {\n"
        "    #^#COMPARISONS#$#\n"
        "}\n";

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::parse::EnumField::Type::Uint64) ||
        (type == commsdsl::parse::EnumField::Type::Uintvar);


    util::StringsList conditions;
    for (auto& l : rangesToProcess) {
        assert(!l.empty());
        auto* condTempl = &VersionBothCondTempl;
        if (l.front().m_sinceVersion == 0U) {
            assert(l.front().m_deprecatedSince != commsdsl::parse::Protocol::notYetDeprecated());
            condTempl = &VersionUntilCondTempl;
        }
        else if (commsdsl::parse::Protocol::notYetDeprecated() <= l.front().m_deprecatedSince) {
            condTempl = &VersionFromCondTempl;
        }

        util::StringsList comparisons;
        for (auto& r : l) {
            static const std::string ValueBothCompTempl =
                "if ((static_cast<typename Base::ValueType>(#^#MIN_VALUE#$#) <= Base::getValue()) &&\n"
                "    (Base::getValue() <= static_cast<typename Base::ValueType>(#^#MAX_VALUE#$#))) {\n"
                "    return true;\n"
                "}";

            static const std::string ValueSingleCompTempl =
                "if (Base::getValue() == static_cast<typename Base::ValueType>(#^#MIN_VALUE#$#)) {\n"
                "    return true;\n"
                "}";


            std::string minValue;
            std::string maxValue;
            if (bigUnsigned) {
                minValue = util::numToString(static_cast<std::uintmax_t>(r.m_min));
                maxValue = util::numToString(static_cast<std::uintmax_t>(r.m_max));
            }
            else {
                minValue = util::numToString(r.m_min);
                maxValue = util::numToString(r.m_max);
            }

            util::ReplacementMap repl = {
                {"MIN_VALUE", std::move(minValue)},
                {"MAX_VALUE", std::move(maxValue)},
            };

            auto* templ = &ValueBothCompTempl;
            if (r.m_min == r.m_max) {
                templ = &ValueSingleCompTempl;
            }
            comparisons.push_back(util::processTemplate(*templ, repl));
        }

        util::ReplacementMap repl = {
            {"COMPARISONS", util::strListToString(comparisons, "\n\n", "")},
            {"FROM_VERSION", util::numToString(l.front().m_sinceVersion)},
            {"UNTIL_VERSION", util::numToString(l.front().m_deprecatedSince)}
        };
        conditions.push_back(util::processTemplate(*condTempl, repl));
    }

    static const std::string Templ =
        "if (Base::valid()) {\n"
        "    return true;\n"
        "}\n\n"
        "#^#CONDITIONS#$#\n"
        "return false;\n"
        ;

    std::string condStr = util::strListToString(conditions, "\n", "");
    util::ReplacementMap repl = {
        {"CONDITIONS", std::move(condStr)}
    };
    return util::processTemplate(Templ, repl);
}

std::size_t CommsEnumField::commsMinLengthImpl() const
{
    if (enumDslObj().availableLengthLimit()) {
        return 1U;
    }

    return CommsBase::commsMinLengthImpl();
}

std::string CommsEnumField::commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const
{
    static_cast<void>(accStr);
    assert(accStr.empty());

    if (value.empty()) {
        return util::numToString(enumDslObj().defaultValue());
    }

    try {
        auto val = static_cast<std::intmax_t>(std::stoll(value, nullptr, 0));
        return util::numToString(val);
    }
    catch (...) {
        // nothing to do
    }    

    auto obj = enumDslObj();
    auto& values = obj.values();
    auto iter = values.find(value);
    if (iter != values.end()) {
        return util::numToString(iter->second.m_value);
    }    

    auto lastDot = value.find_last_of(".");
    if (lastDot == std::string::npos) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return CommsBase::commsCompPrepValueStrImpl(accStr, value);
    }

    auto* otherEnum = generator().findField(std::string(value, 0, lastDot));
    if ((otherEnum == nullptr) || (otherEnum->dslObj().kind() != commsdsl::parse::Field::Kind::Enum)) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return CommsBase::commsCompPrepValueStrImpl(accStr, value);
    }

    auto& castedOtherEnum = static_cast<const CommsEnumField&>(*otherEnum);
    auto& otherValues = castedOtherEnum.enumDslObj().values();
    auto otherIter = otherValues.find(std::string(value, lastDot + 1));
    if (otherIter == otherValues.end()) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return CommsBase::commsCompPrepValueStrImpl(accStr, value);
    }    

    return util::numToString(otherIter->second.m_value);
}

bool CommsEnumField::commsPrepareValidRangesInternal()
{
    auto obj = enumDslObj();

    auto& gen = generator();
    bool validCheckVersion =
        gen.schemaOf(*this).versionDependentCode() &&
        obj.validCheckVersion();

    auto& values = obj.values();
    m_validRanges.reserve(values.size());
    for (auto& v : values) {
        bool exists =
            gen.doesElementExist(
                v.second.m_sinceVersion,
                v.second.m_deprecatedSince,
                !validCheckVersion);
        if (!exists) {
            continue;
        }

        m_validRanges.emplace_back();
        auto& r = m_validRanges.back();
        r.m_min = v.second.m_value;
        r.m_max = r.m_min;
        r.m_sinceVersion = v.second.m_sinceVersion;
        r.m_deprecatedSince = v.second.m_deprecatedSince;
    }

    if (m_validRanges.empty()) {
        gen.logger().error("Enum \"" + obj.name() + "\" doesn't define any value");
        return false;
    }

    if (m_validRanges.size() <= 1U) {
        return true;
    }

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::parse::EnumField::Type::Uint64) ||
        (type == commsdsl::parse::EnumField::Type::Uintvar);


    // Sort
    std::sort(
        m_validRanges.begin(), m_validRanges.end(),
        [bigUnsigned, validCheckVersion](auto& e1, auto& e2)
        {
            if (validCheckVersion) {
                if (e1.m_sinceVersion != e2.m_sinceVersion) {
                    return e1.m_sinceVersion < e2.m_sinceVersion;
                }

                if (e1.m_deprecatedSince != e2.m_deprecatedSince) {
                    return e1.m_deprecatedSince < e2.m_deprecatedSince;
                }
            }

            if (bigUnsigned) {
                return static_cast<std::uintmax_t>(e1.m_min) < (static_cast<std::uintmax_t>(e2.m_min));
            }

            return e1.m_min < e2.m_min;
        });

    // Merge
    for (auto iter = m_validRanges.begin(); iter != m_validRanges.end(); ++iter) {
        if (iter->m_deprecatedSince == 0U) {
            continue;
        }

        for (auto nextIter = iter + 1; nextIter != m_validRanges.end(); ++nextIter) {
            if (nextIter->m_deprecatedSince == 0U) {
                continue;
            }

            if (validCheckVersion) {
                if ((iter->m_sinceVersion != nextIter->m_sinceVersion) ||
                    (iter->m_deprecatedSince != nextIter->m_deprecatedSince)) {
                    break;
                }
            }

            if ((iter->m_max + 1) < nextIter->m_min) {
                break;
            }

            assert(iter->m_min <= nextIter->m_min);
            nextIter->m_deprecatedSince = 0U; // invalidate next range
            iter->m_max = std::max(iter->m_max, nextIter->m_max);
        }
    }

    // Remove invalid
    m_validRanges.erase(
        std::remove_if(
            m_validRanges.begin(), m_validRanges.end(),
            [](auto& elem)
            {
                return elem.m_deprecatedSince == 0U;
            }),
        m_validRanges.end());
    return true;
}

bool CommsEnumField::commsIsDirectValueNameMappingInternal() const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    assert(!revValues.empty());
    auto firstIter = revValues.begin();
    if (firstIter->first < 0) {
        // has negative numbers
        return false;
    }

    auto lastIter = revValues.end();
    std::advance(lastIter, -1);
    auto lastVal = static_cast<std::uintmax_t>(lastIter->first);
    auto maxDirectAllowed = static_cast<std::size_t>((revValues.size() * 110U) / 100);
    if ((maxDirectAllowed <= lastVal) && (10 <= lastVal)) {
        // Too sparse
        return false;
    }

    return true;    
}

std::string CommsEnumField::commsCommonEnumInternal() const
{
    auto& gen = generator();
    if (dslObj().semanticType() == commsdsl::parse::Field::SemanticType::MessageId) {
        static const std::string Templ =
            "/// @brief Values enumerator for\n"
            "///     @ref #^#SCOPE#$# field.\n"
            "using ValueType = #^#MSG_ID#$#;\n";

        util::ReplacementMap repl = {
            {"SCOPE", comms::scopeFor(*this, gen)},
            {"MSG_ID", comms::scopeForRoot(strings::msgIdEnumNameStr(), gen)}
        };
        return util::processTemplate(Templ, repl);
    }

    static const std::string Templ =
        "/// @brief Values enumerator for\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "enum class ValueType : #^#TYPE#$#\n"
        "{\n"
        "    #^#VALUES#$#\n"
        "};\n"
    ;    

    auto values = commsEnumValues();
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, gen)},
        {"TYPE", comms::cppIntTypeFor(enumDslObj().type(), enumDslObj().maxLength())},
        {"VALUES", util::strListToString(values, "\n", "")}
    };
    return util::processTemplate(Templ, repl);        
}

std::string CommsEnumField::commsCommonValueNameMapInternal() const
{
    static const std::string Templ = 
        "/// @brief Single value name info entry\n"
        "using ValueNameInfo = #^#NAME_INFO_TYPE#$#;\n\n"
        "/// @brief Type returned from @ref valueNamesMap() member function.\n"
        "/// @details The @b first value of the pair is pointer to the map array,\n"
        "///     The @b second value of the pair is the size of the array.\n"
        "using ValueNamesMapInfo = std::pair<const ValueNameInfo*, std::size_t>;\n";

    util::ReplacementMap repl = {
        {"NAME_INFO_TYPE", commsIsDirectValueNameMappingInternal() ? "const char*" : "std::pair<ValueType, const char*>"}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsCommonValueNameFuncCodeInternal() const
{
    std::string body;
    if (commsIsDirectValueNameMappingInternal()) {
        body = commsCommonValueNameDirectBodyInternal();
    }
    else {
        body = commsCommonValueNameBinSearchBodyInternal();
    }
    assert(!body.empty());

    static const std::string Templ = 
        "/// @brief Retrieve name of the enum value\n"
        "static const char* valueName(ValueType val)\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";


    util::ReplacementMap repl = {
        {"BODY", std::move(body)}
    };
    return util::processTemplate(Templ, repl);
}

const std::string& CommsEnumField::commsCommonValueNameDirectBodyInternal() const
{
    static const std::string Templ = 
        "auto namesMapInfo = valueNamesMap();\n"
        "if (namesMapInfo.second <= static_cast<std::size_t>(val)) {\n"
        "    return nullptr;\n"
        "}\n\n"
        "return namesMapInfo.first[static_cast<std::size_t>(val)];";


    return Templ;    
}

const std::string& CommsEnumField::commsCommonValueNameBinSearchBodyInternal() const
{
    static const std::string Templ = 
        "auto namesMapInfo = valueNamesMap();\n"
        "auto begIter = namesMapInfo.first;\n"
        "auto endIter = begIter + namesMapInfo.second;\n"
        "auto iter = std::lower_bound(\n"
        "    begIter, endIter, val,\n"
        "    [](const ValueNameInfo& info, ValueType v) -> bool\n"
        "    {\n"
        "        return info.first < v;\n"
        "    });\n\n"
        "if ((iter == endIter) || (iter->first != val)) {\n"
        "    return nullptr;\n"
        "}\n\n"
        "return iter->second;";


    return Templ;
}

std::string CommsEnumField::commsCommonValueNamesMapFuncCodeInternal() const
{
    std::string body;
    if (commsIsDirectValueNameMappingInternal()) {
        body = commsCommonValueNamesMapDirectBodyInternal();
    }
    else {
        body = commsCommonValueNamesMapBinSearchBodyInternal();
    }
    assert(!body.empty());

    static const std::string Templ = 
        "/// @brief Retrieve map of enum value names\n"
        "static ValueNamesMapInfo valueNamesMap()\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";


    util::ReplacementMap repl = {
        {"BODY", std::move(body)}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsCommonValueNamesMapDirectBodyInternal() const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    auto& values = obj.values();
    assert(!revValues.empty());
    auto& thisSchema = generator().schemaOf(*this);

    std::intmax_t nextValue = 0;
    StringsList names;
    for (auto& v : revValues) {
        if (v.first < nextValue) {
            continue;
        }

        while (nextValue < v.first) {
            names.push_back("nullptr");
            ++nextValue;
        }

        auto getDisplayNameFunc = 
            [](auto& infoPair) -> const std::string&
            {
                if (infoPair.second.m_displayName.empty()) {
                    return infoPair.first;
                }

                if (infoPair.second.m_displayName == "_") {
                    return strings::emptyString();
                }

                return infoPair.second.m_displayName;
            };

        ++nextValue;
        auto addElementNameFunc = 
            [&names, getDisplayNameFunc](auto& infoPair) 
            {
                names.push_back('\"' + getDisplayNameFunc(infoPair) + '\"');
            };

        auto valIter = values.find(v.second);
        assert(valIter != values.end());
        if ((!obj.isNonUniqueAllowed()) ||
            (thisSchema.schemaVersion() < valIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*valIter);
            continue;
        }

        auto allRevValues = revValues.equal_range(v.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevValues.first; iter != allRevValues.second; ++iter) {
            auto vIter = values.find(iter->second);
            assert(vIter != values.end());
            if (thisSchema.schemaVersion() < vIter->second.m_deprecatedSince) {
                addElementNameFunc(*vIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*valIter);
    }

    static const std::string Templ = 
        "static const char* Map[] = {\n"
        "    #^#NAMES#$#\n"
        "};\n"
        "static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "return std::make_pair(&Map[0], MapSize);";


    util::ReplacementMap repl = {
        {"NAMES", util::strListToString(names, ",\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsCommonValueNamesMapBinSearchBodyInternal() const
{
    auto obj = enumDslObj();
    auto type = obj.type();

    std::string names;
    if ((type == commsdsl::parse::EnumField::Type::Uint64) ||
        ((type == commsdsl::parse::EnumField::Type::Uintvar) && (sizeof(std::uint64_t) <= obj.maxLength()))) {
        names = commsCommonBigUnsignedValueNameBinSearchPairsInternal();
    }
    else {
        names = commsCommonValueNameBinSearchPairsInternal();
    }

    static const std::string Templ = 
        "static const ValueNameInfo Map[] = {\n"
        "    #^#NAMES#$#\n"
        "};\n"
        "static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "return std::make_pair(&Map[0], MapSize);";


    util::ReplacementMap repl = {
        {"NAMES", std::move(names)}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsCommonBigUnsignedValueNameBinSearchPairsInternal() const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    assert(!revValues.empty());

    std::vector<std::uintmax_t> valuesSeq;
    valuesSeq.reserve(revValues.size());
    for (auto& v : revValues) {
        valuesSeq.push_back(static_cast<std::uintmax_t>(v.first));
    }
    std::sort(valuesSeq.begin(), valuesSeq.end());

    auto& currSchema = generator().schemaOf(*this);
    auto& values = obj.values();
    bool firstElem = true;
    std::intmax_t lastValue = std::numeric_limits<std::intmax_t>::min();
    StringsList names;
    for (auto& posV : valuesSeq) {
        auto castedV = static_cast<std::intmax_t>(posV);
        if ((!firstElem) && (lastValue == castedV)) {
            continue;
        }

        auto revIter = revValues.find(castedV);
        assert(revIter != revValues.end());
        auto& v = *revIter;

        auto getDisplayNameFunc = 
            [](auto& infoPair) -> const std::string&
            {
                if (infoPair.second.m_displayName.empty()) {
                    return infoPair.first;
                }

                if (infoPair.second.m_displayName == "_") {
                    return strings::emptyString();
                }

                return infoPair.second.m_displayName;
            };

        auto addElementNameFunc = 
            [&names, &v, &firstElem, &lastValue, getDisplayNameFunc](auto& infoPair)
            {
                auto str = 
                    "std::make_pair(ValueType::" +
                    v.second + ", \"" + getDisplayNameFunc(infoPair) +
                    "\")";
                names.push_back(std::move(str));
                firstElem = false;
                lastValue = v.first;
            };

        auto valIter = values.find(v.second);
        assert(valIter != values.end());
        if ((!obj.isNonUniqueAllowed()) ||
            (currSchema.schemaVersion() < valIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*valIter);
            continue;
        }

        auto allRevValues = revValues.equal_range(v.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevValues.first; iter != allRevValues.second; ++iter) {
            auto vIter = values.find(iter->second);
            assert(vIter != values.end());
            if (currSchema.schemaVersion() < vIter->second.m_deprecatedSince) {
                addElementNameFunc(*vIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*valIter);
    }

    return util::strListToString(names, ",\n", "");
}

std::string CommsEnumField::commsCommonValueNameBinSearchPairsInternal() const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    auto& values = obj.values();
    assert(!revValues.empty());
    bool isMessageId =
        obj.semanticType() == commsdsl::parse::Field::SemanticType::MessageId;    

    bool firstElem = true;
    std::intmax_t lastValue = std::numeric_limits<std::intmax_t>::min();
    StringsList names;
    auto& currSchema = generator().schemaOf(*this);
    for (auto& v : revValues) {
        if ((!firstElem) && (lastValue == v.first)) {
            continue;
        }

        auto getDisplayNameFunc = 
            [](auto& infoPair) -> const std::string&
            {
                if (infoPair.second.m_displayName.empty()) {
                    return infoPair.first;
                }

                if (infoPair.second.m_displayName == "_") {
                    return strings::emptyString();
                }

                return infoPair.second.m_displayName;
            };

        auto getValueStrFunc = 
            [isMessageId, &currSchema](const std::string& s)
            {
                if (isMessageId) {
                    return currSchema.mainNamespace() + "::" + strings::msgIdPrefixStr() + s;
                }

                return "ValueType::" + s;
            };

        auto addElementNameFunc = 
            [&names, &v, &firstElem, &lastValue, getDisplayNameFunc, &getValueStrFunc](auto& infoPair) 
            {
                auto str = 
                    "std::make_pair(" +
                    getValueStrFunc(v.second) +
                    ", \"" + getDisplayNameFunc(infoPair) +
                    "\")";
                names.push_back(std::move(str));
                firstElem = false;
                lastValue = v.first;
            };

        auto valIter = values.find(v.second);
        assert(valIter != values.end());
        if ((!obj.isNonUniqueAllowed()) ||
            (currSchema.schemaVersion() < valIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*valIter);
            continue;
        }

        auto allRevValues = revValues.equal_range(v.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevValues.first; iter != allRevValues.second; ++iter) {
            auto vIter = values.find(iter->second);
            assert(vIter != values.end());
            if (currSchema.schemaVersion() < vIter->second.m_deprecatedSince) {
                addElementNameFunc(*vIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*valIter);
    }

    return util::strListToString(names, ",\n", "");
}

std::string CommsEnumField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddDefaultValueOptInternal(opts);
    commsAddLengthOptInternal(opts);
    commsAddValidRangesOptInternal(opts);
    commsAddAvailableLengthLimitOptInternal(opts);

    return util::strListToString(opts, ",\n", "");
}

std::string CommsEnumField::commsDefValueNameMapInternal() const
{
    static const std::string Templ = 
        "/// @brief Single value name info entry.\n"
        "using ValueNameInfo = #^#COMMON_SCOPE#$#::ValueNameInfo;\n\n"
        "/// @brief Type returned from @ref valueNamesMap() member function.\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::ValueNamesMapInfo.\n"
        "using ValueNamesMapInfo = #^#COMMON_SCOPE#$#::ValueNamesMapInfo;\n";

    util::ReplacementMap repl = {
        {"COMMON_SCOPE", comms::commonScopeFor(*this, generator())}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsDefValueNameFuncCodeInternal() const
{
    static const std::string Templ = 
        "/// @brief Retrieve name of the enum value.\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::valueName().\n"
        "static const char* valueName(ValueType val)\n"
        "{\n"
        "    return #^#COMMON_SCOPE#$#::valueName(val);\n"
        "}\n\n"
        "/// @brief Retrieve name of the enum value.\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::valueName().\n"
        "static const char* valueNameOf(ValueType val)\n"
        "{\n"
        "    return #^#COMMON_SCOPE#$#::valueName(val);\n"
        "}\n\n"        
        "/// @brief Retrieve name of the @b current value\n"
        "const char* valueName() const\n"
        "{\n"
        "    return valueName(Base::getValue());\n"
        "}\n";        

    util::ReplacementMap repl = {
        {"COMMON_SCOPE", comms::commonScopeFor(*this, generator())}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsEnumField::commsDefValueNamesMapFuncCodeInternal() const
{
    static const std::string Templ = 
        "/// @brief Retrieve map of enum value names.\n"
        "/// @see @ref #^#COMMON_SCOPE#$#::valueNamesMap().\n"
        "static ValueNamesMapInfo valueNamesMap()\n"
        "{\n"
        "    return #^#COMMON_SCOPE#$#::valueNamesMap();\n"
        "}\n";


    util::ReplacementMap repl = {
        {"COMMON_SCOPE", comms::commonScopeFor(*this, generator())}
    };
    return util::processTemplate(Templ, repl);
}

void CommsEnumField::commsAddDefaultValueOptInternal(StringsList& opts) const
{
    auto obj = enumDslObj();
    auto defaultValue = obj.defaultValue();

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.type();
    if ((defaultValue < 0) &&
        ((type == commsdsl::parse::EnumField::Type::Uint64) || (type == commsdsl::parse::EnumField::Type::Uintvar))) {
        auto str =
            "comms::option::def::DefaultBigUnsignedNumValue<" +
            util::numToString(static_cast<std::uintmax_t>(defaultValue)) +
            '>';
        opts.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::def::DefaultNumValue<" +
        util::numToString(defaultValue) +
        '>';
    opts.push_back(std::move(str));
}

void CommsEnumField::commsAddLengthOptInternal(StringsList& opts) const
{
    auto obj = enumDslObj();
    auto type = obj.type();

    if ((type == commsdsl::parse::EnumField::Type::Intvar) ||
        (type == commsdsl::parse::EnumField::Type::Uintvar)) {
        auto str =
            "comms::option::def::VarLength<" +
            util::numToString(obj.minLength()) +
            ", " +
            util::numToString(obj.maxLength()) +
            '>';
        opts.push_back(std::move(str));
        return;
    }

    auto bitLength = obj.bitLength();
    if (bitLength != 0U) {
        opts.push_back("comms::option::def::FixedBitLength<" + util::numToString(bitLength) + '>');
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
        auto str =
            "comms::option::def::FixedLength<" +
            util::numToString(obj.minLength()) +
            '>';
        opts.push_back(std::move(str));
    }
}

void CommsEnumField::commsAddValidRangesOptInternal(StringsList& opts) const
{
    auto obj = enumDslObj();

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::parse::EnumField::Type::Uint64) ||
        ((type != commsdsl::parse::EnumField::Type::Uintvar) && (obj.maxLength() >= sizeof(std::int64_t)));

    bool validCheckVersion =
        generator().schemaOf(*this).versionDependentCode() &&
        obj.validCheckVersion();

    auto addOptFunc =
        [&opts, bigUnsigned](auto& r)
        {
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

                opts.push_back(std::move(str));
                return;
            } while (false);

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

            opts.push_back(std::move(str));
        };

    assert(!m_validRanges.empty());
    if (!validCheckVersion) {
        if (MaxRangesInOpts < m_validRanges.size()) {
            return; // Will be in validity check
        }

        for (auto& range : m_validRanges) {
            addOptFunc(range);
        }
        return;
    }

    auto uncondStartIter =
        std::find_if(
            m_validRanges.begin(), m_validRanges.end(),
            [](auto& elem)
            {
                return
                    (elem.m_sinceVersion == 0U) &&
                    (elem.m_deprecatedSince == commsdsl::parse::Protocol::notYetDeprecated());
            });

    if (uncondStartIter == m_validRanges.end()) {
        // No unconditionals found;
        opts.push_back("comms::option::def::VersionStorage");
        opts.push_back("comms::option::def::InvalidByDefault");
        return;
    }

    auto uncondEndIter =
        std::find_if(
            uncondStartIter + 1, m_validRanges.end(),
            [](auto& elem)
            {
                return
                    (elem.m_sinceVersion != 0U) ||
                    (elem.m_deprecatedSince != commsdsl::parse::Protocol::notYetDeprecated());
            });

    auto uncondCount =
        static_cast<std::size_t>(std::distance(uncondStartIter, uncondEndIter));

    if (uncondCount != m_validRanges.size()) {
        opts.push_back("comms::option::def::VersionStorage");
    }

    for (auto iter = uncondStartIter; iter != uncondEndIter; ++iter) {
        addOptFunc(*iter);
    }
}

void CommsEnumField::commsAddAvailableLengthLimitOptInternal(StringsList& opts) const
{
    if (enumDslObj().availableLengthLimit()) {
        util::addToStrList("comms::option::def::AvailableLengthLimit", opts);
    }
}

} // namespace commsdsl2comms
