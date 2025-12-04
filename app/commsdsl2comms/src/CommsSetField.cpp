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

#include "CommsSetField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2comms
{

namespace
{

const auto CommsMaxBits = std::numeric_limits<std::uintmax_t>::digits;

} // namespace

CommsSetField::CommsSetField(CommsGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    CommsBase(static_cast<GenBase&>(*this))
{
}

bool CommsSetField::genPrepareImpl()
{
    return
        GenBase::genPrepareImpl() &&
        commsPrepare();
}

bool CommsSetField::genWriteImpl() const
{
    return commsWrite();
}

CommsSetField::CommsIncludesList CommsSetField::commsCommonIncludesImpl() const
{
    CommsIncludesList result = {
        "<type_traits>"
    };

    return result;
}

std::string CommsSetField::commsCommonCodeBodyImpl() const
{
    static const std::string Templ = {
        "#^#NAME_FUNC#$#\n"
        "#^#BIT_NAME_FUNC#$#\n"
    };

    util::GenReplacementMap repl = {
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"BIT_NAME_FUNC", commsCommonBitNameFuncCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

CommsSetField::CommsIncludesList CommsSetField::commsDefIncludesImpl() const
{
    CommsIncludesList result = {
        "comms/field/BitmaskValue.h"
    };

    return result;
}

std::string CommsSetField::commsDefBaseClassImpl() const
{
    static const std::string Templ =
        "comms::field::BitmaskValue<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";

    auto& gen = genGenerator();
    auto parseObj = genSetFieldParseObj();
    util::GenReplacementMap repl = {
        {"PROT_NAMESPACE", gen.genSchemaOf(*this).genMainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(parseObj.parseEndian())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsSetField::commsDefPublicCodeImpl() const
{
    static const std::string Templ =
        "#^#BITS_ACCESS#$#\n"
        "#^#BIT_NAME#$#";

    util::GenReplacementMap repl = {
        {"BITS_ACCESS", commsDefBitsAccessCodeInternal()},
        {"BIT_NAME", commsDefBitNameFuncCodeInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsSetField::commsDefValidFuncBodyImpl() const
{
    auto obj = genSetFieldParseObj();
    auto& gen = genGenerator();

    bool validCheckVersion =
        gen.genSchemaOf(*this).genVersionDependentCode() &&
        obj.parseValidCheckVersion();

    if (!validCheckVersion) {
        return strings::genEmptyString();
    }

    using Key = std::tuple<unsigned, unsigned>;
    struct Value
    {
        std::uintmax_t m_reservedMask = 0U;
        std::uintmax_t m_reservedValue = 0U;
    };

    using Map = std::map<Key, Value>;
    Map bitsToCheck;

    std::uintmax_t allReservedMask = 0U;
    std::uintmax_t repeatingReservedMask = 0U;
    std::vector<unsigned> repeatingBits;
    auto& bits = obj.parseBits();
    for (auto& b : bits) {
        if ((b.second.m_sinceVersion <= obj.parseSinceVersion()) &&
            (obj.parseDeprecatedSince() <= b.second.m_deprecatedSince)) {
            continue;
        }

        if (!gen.genDoesElementExist(b.second.m_sinceVersion, b.second.m_deprecatedSince, false)) {
            continue; // already handled
        }

        assert(!b.second.m_reserved); // Should be handled in options

        auto key = std::make_tuple(b.second.m_sinceVersion, b.second.m_deprecatedSince);
        auto& elem = bitsToCheck[key];

        auto bitMask = static_cast<std::uintmax_t>(1U) << b.second.m_idx;
        elem.m_reservedMask |= bitMask;

        if (b.second.m_reservedValue) {
            elem.m_reservedValue |= bitMask;
        }

        if (((allReservedMask & bitMask) != 0U) &&
            ((repeatingReservedMask & bitMask) == 0U)) {
            repeatingReservedMask |= bitMask;
            repeatingBits.push_back(b.second.m_idx);
        }

        allReservedMask |= bitMask;
    }

    if (repeatingReservedMask != 0U) {
        for (auto& b : bitsToCheck) {
            b.second.m_reservedMask &= ~repeatingReservedMask;
            b.second.m_reservedValue &= ~repeatingReservedMask;
        }
    }

    util::GenStringsList conditions;
    for (auto& info : bitsToCheck) {
        if (info.second.m_reservedMask == 0U) {
            continue;
        }

            static const std::string VersionBothCondTempl =
                "if (((Base::getVersion() < #^#FROM_VERSION#$#) || (#^#UNTIL_VERSION#$# <= Base::getVersion())) && \n"
                "    ((Base::getValue() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

            static const std::string VersionFromCondTempl =
                "if ((Base::getVersion() < #^#FROM_VERSION#$#) &&\n"
                "    ((Base::getValue() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

            static const std::string VersionUntilCondTempl =
                "if ((#^#UNTIL_VERSION#$# <= Base::getVersion()) &&\n"
                "    ((Base::getValue() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

        auto* condTempl = &VersionBothCondTempl;
        if (std::get<0>(info.first) == 0U) {
            assert(std::get<1>(info.first) != commsdsl::parse::ParseProtocol::parseNotYetDeprecated());
            condTempl = &VersionUntilCondTempl;
        }
        else if (commsdsl::parse::ParseProtocol::parseNotYetDeprecated() <= std::get<1>(info.first)) {
            condTempl = &VersionFromCondTempl;
        }

        util::GenReplacementMap repl = {
            {"BITS_MASK", util::genNumToString(info.second.m_reservedMask, true)},
            {"VALUE_MASK", util::genNumToString(info.second.m_reservedValue, true)},
            {"FROM_VERSION", util::genNumToString(std::get<0>(info.first))},
            {"UNTIL_VERSION", util::genNumToString(std::get<1>(info.first))},
        };
        conditions.push_back(util::genProcessTemplate(*condTempl, repl));
    }

    for (auto idx : repeatingBits) {
        auto elems = obj.parseRevBits().equal_range(idx);
        assert(elems.first != elems.second);
        std::vector<std::pair<unsigned, unsigned> > versionRanges;
        for (auto iter = elems.first; iter != elems.second; ++iter) {
            auto& bitName = iter->second;
            auto bitIter = bits.find(bitName);
            if (bitIter == bits.end()) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                continue;
            }

            versionRanges.push_back(std::make_pair(bitIter->second.m_sinceVersion, bitIter->second.m_deprecatedSince));
        }

        std::sort(
            versionRanges.begin(), versionRanges.end(),
            [](auto& e1, auto& e2)
            {
                if (e1.first != e2.first) {
                    return e1.first < e2.first;
                }

                return e1.second < e2.second;
            });

        unsigned prev = 0U;
        for (auto& r : versionRanges) {
            auto newPrev = r.second;
            r.second = r.first;
            r.first = prev;
            prev = newPrev;
        }

        if (prev < obj.parseDeprecatedSince()) {
            versionRanges.push_back(std::make_pair(prev, obj.parseDeprecatedSince()));
        }

        // unify ranges;
        assert(!versionRanges.empty());
        for (auto rIdx = 0U; rIdx < versionRanges.size(); ++rIdx) {
            auto& thisRange = versionRanges[rIdx];
            if ((thisRange.second == 0) || (thisRange.first == thisRange.second)) {
                continue;
            }

            for (auto nextIdx = rIdx + 1; nextIdx < versionRanges.size(); ++nextIdx) {
                auto& nextRange = versionRanges[nextIdx];
                if ((nextRange.second == 0) || (nextRange.first == nextRange.second)) {
                    continue;
                }

                if (thisRange.second < nextRange.first) {
                    break;
                }

                thisRange.second = nextRange.second;
                nextRange.second = 0U; // invalidate

            }
        }

        util::GenStringsList extraConds;
        for (auto& r : versionRanges) {
            if ((r.second == 0U) || (r.first == r.second)) {
                continue; // ignore invalid ranges
            }

            static const std::string Templ =
                "if ((#^#FROM_VERSION#$# <= Base::getVersion()) &&\n"
                "    (Base::getVersion() < #^#UNTIL_VERSION#$#)) {\n"
                "    return false;\n"
                "}";

            util::GenReplacementMap repl = {
                {"FROM_VERSION", util::genNumToString(r.first)},
                {"UNTIL_VERSION", util::genNumToString(r.second)},
            };

            extraConds.push_back(util::genProcessTemplate(Templ, repl));
        }

        static const std::string Templ =
            "if ((Base::getValue() & #^#RESERVED_MASK#$#) != #^#RESERVED_VALUE#$#) {\n"
            "    #^#CONDITIONS#$#\n"
            "}\n";

        auto bitMask = static_cast<std::uintmax_t>(1U) << idx;
        std::uintmax_t bitValue = 0U;

        auto& bitInfoName = elems.first->second;
        auto bitInfoIter = bits.find(bitInfoName);
        assert(bitInfoIter != bits.end());
        if (bitInfoIter->second.m_reservedValue) {
            bitValue |= bitMask;
        }

        if (!extraConds.empty()) {
            util::GenReplacementMap repl = {
                {"RESERVED_MASK", util::genNumToString(bitMask, true)},
                {"RESERVED_VALUE", util::genNumToString(bitValue, true)},
                {"CONDITIONS", util::genStrListToString(extraConds, "\n\n", "")}
            };

            conditions.push_back(util::genProcessTemplate(Templ, repl));
        }
    }

    if (conditions.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "if (!Base::valid()) {\n"
        "    return false;\n"
        "}\n\n"
        "#^#CONDITIONS#$#\n"
        "return true;\n"
        ;

    util::GenReplacementMap repl = {
        {"CONDITIONS", util::genStrListToString(conditions, "\n", "")}
    };
    return util::genProcessTemplate(Templ, repl);
}

bool CommsSetField::commsIsVersionDependentImpl() const
{
    assert(genGenerator().genSchemaOf(*this).genVersionDependentCode());
    auto obj = genSetFieldParseObj();
    if (!obj.parseValidCheckVersion()) {
        return false;
    }

    auto& bits = obj.parseBits();

    unsigned minVersion = obj.parseSinceVersion();
    unsigned maxVersion = obj.parseDeprecatedSince();
    auto iter =
        std::find_if(
            bits.begin(), bits.end(),
            [minVersion, maxVersion](auto& elem)
            {
                return
                    (minVersion < elem.second.m_sinceVersion) ||
                    (elem.second.m_deprecatedSince < maxVersion);
            });

    return (iter != bits.end());
}

std::size_t CommsSetField::commsMinLengthImpl() const
{
    if (genSetFieldParseObj().parseAvailableLengthLimit()) {
        return 1U;
    }

    return CommsBase::commsMinLengthImpl();
}

std::string CommsSetField::commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const
{
    if (accStr.empty()) {
        return CommsBase::commsValueAccessStrImpl(accStr, prefix);
    }

    auto& bits = genSetFieldParseObj().parseBits();
    auto iter = bits.find(accStr);
    if (iter == bits.end()) {
        genGenerator().genLogger().genError("Failed to find bit reference " + accStr + " for field " + comms::genScopeFor(*this, genGenerator()));
        assert(false);
        return strings::genUnexpectedValueStr();
    }

    return prefix + ".getBitValue_" + accStr + "()";
}

bool CommsSetField::commsVerifyInnerRefImpl(const std::string& refStr) const
{
    auto& bits = genSetFieldParseObj().parseBits();
    return bits.find(refStr) != bits.end();
}

std::string CommsSetField::commsCommonBitNameFuncCodeInternal() const
{
    auto obj = genSetFieldParseObj();
    auto& bits = obj.parseBits();
    auto& revBits = obj.parseRevBits();
    std::intmax_t nextBit = 0;
    GenStringsList names;
    for (auto& b : revBits) {
        if (b.first < nextBit) {
            continue;
        }

        while (nextBit < b.first) {
            names.push_back("nullptr");
            ++nextBit;
        }

        auto getDisplayNameFunc =
            [](auto& infoPair) -> const std::string&
            {
                if (infoPair.second.m_displayName.empty()) {
                    return infoPair.first;
                }

                if (infoPair.second.m_displayName == "_") {
                    return strings::genEmptyString();
                }

                return infoPair.second.m_displayName;
            };

        ++nextBit;
        auto addElementNameFunc =
            [&names, getDisplayNameFunc](auto& infoPair)
            {
                names.push_back('\"' + getDisplayNameFunc(infoPair) + '\"');
            };

        auto bitIter = bits.find(b.second);
        assert(bitIter != bits.end());
        if ((!obj.parseIsNonUniqueAllowed()) ||
            (genGenerator().genSchemaOf(*this).genSchemaVersion() < bitIter->second.m_deprecatedSince) ||
            (obj.parseIsUnique())) {
            addElementNameFunc(*bitIter);
            continue;
        }

        auto allRevBits = revBits.equal_range(b.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevBits.first; iter != allRevBits.second; ++iter) {
            auto bIter = bits.find(iter->second);
            assert(bIter != bits.end());
            if (genGenerator().genSchemaOf(*this).genSchemaVersion() < bIter->second.m_deprecatedSince) {
                addElementNameFunc(*bIter);
                foundNotDeprecated = true;
                break;
            }
        }

        if (foundNotDeprecated) {
            continue;
        }

        addElementNameFunc(*bitIter);
    }

    std::string body;
    do {
        if (names.empty()) {
            body =
                "static_cast<void>(idx);\n"
                "return nullptr;";
            break;
        }

        static const std::string BodyTempl =
            "static const char* Map[] = {\n"
            "    #^#NAMES#$#\n"
            "};\n\n"
            "static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n"
            "if (MapSize <= idx) {\n"
            "    return nullptr;\n"
            "}\n\n"
            "return Map[idx];";

        util::GenReplacementMap bodyRepl = {
            {"NAMES", util::genStrListToString(names, ",\n", "")},
        };

        body = util::genProcessTemplate(BodyTempl, bodyRepl);
    } while (false);

    static const std::string Templ =
        "/// @brief Retrieve name of the bit of\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "static const char* bitName(std::size_t idx)\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::GenReplacementMap repl = {
        {"BODY", std::move(body)},
        {"SCOPE", comms::genScopeFor(*this, genGenerator())}
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsSetField::commsDefFieldOptsInternal() const
{
    util::GenStringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddLengthOptInternal(opts);
    commsAddDefaultValueOptInternal(opts);
    commsAddReservedBitsOptInternal(opts);
    commsAddAvailableLengthLimitOptInternal(opts);

    return util::genStrListToString(opts, ",\n", "");
}

std::string CommsSetField::commsDefBitsAccessCodeInternal() const
{
    auto obj = genSetFieldParseObj();
    auto& bits = obj.parseBits();

    std::uintmax_t usedBits = 0U;
    util::GenStringsList names;

    std::map<std::string, unsigned> deprecatedBits;
    for (auto& bitInfo : obj.parseRevBits()) {
        auto idx = bitInfo.first;
        if (CommsMaxBits <= idx) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        auto mask = static_cast<std::uintmax_t>(1) << idx;
        usedBits |= mask;
        names.push_back(bitInfo.second);

        auto iter = bits.find(bitInfo.second);
        assert(iter != bits.end());
        if (genGenerator().genIsElementDeprecated(iter->second.m_deprecatedSince)) {
            deprecatedBits.insert(std::make_pair(bitInfo.second, iter->second.m_deprecatedSince));
        }
    }

    auto getDeprecatedStr =
        [&deprecatedBits](const std::string& n) -> std::string
        {
            auto iter = deprecatedBits.find(n);
            if (iter == deprecatedBits.end()) {
                return strings::genEmptyString();
            }

            return
                "\n"
                "///          Deprecated since version " + std::to_string(iter->second) + '.';
        };

    if (obj.parseIsUnique() && (((usedBits + 1) & usedBits) == 0U)) {
        // sequential
        static const std::string Templ =
            "/// @brief Provides names and generates access functions for internal bits.\n"
            "/// @details See definition of @b COMMS_BITMASK_BITS_SEQ macro\n"
            "///     related to @b comms::field::BitmaskValue class from COMMS library\n"
            "///     for details.\n"
            "///\n"
            "///      The generated enum values and functions are:\n"
            "#^#ACCESS_DOC#$#\n"
            "COMMS_BITMASK_BITS_SEQ(\n"
            "    #^#NAMES#$#\n"
            ");\n";

        util::GenStringsList accessDoc;
        accessDoc.reserve(names.size());
        std::transform(
            names.begin(), names.end(), std::back_inserter(accessDoc),
            [&getDeprecatedStr](auto& n)
            {
                return
                    "///      @li @b BitIdx_" + n + ", @b getBitValue_" + n +
                    "() and @b setBitValue_" + n + "()." +
                    getDeprecatedStr(n);
            });

        util::GenReplacementMap repl = {
            {"ACCESS_DOC", util::genStrListToString(accessDoc, "\n", "")},
            {"NAMES", util::genStrListToString(names, ",\n", "")}
        };
        return util::genProcessTemplate(Templ, repl);
    }

    static const std::string Templ =
        "/// @brief Provide names for internal bits.\n"
        "/// @details See definition of @b COMMS_BITMASK_BITS macro\n"
        "///     related to @b comms::field::BitmaskValue class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///      The generated enum values:\n"
        "#^#BITS_DOC#$#\n"
        "COMMS_BITMASK_BITS(\n"
        "    #^#BITS#$#\n"
        ");\n\n"
        "/// @brief Generates independent access functions for internal bits.\n"
        "/// @details See definition of @b COMMS_BITMASK_BITS_ACCESS macro\n"
        "///     related to @b comms::field::BitmaskValue class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated access functions are:\n"
        "#^#ACCESS_DOC#$#\n"
        "COMMS_BITMASK_BITS_ACCESS(\n"
        "    #^#NAMES#$#\n"
        ");\n";

    util::GenStringsList bitsDoc;
    bitsDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsDoc),
        [&getDeprecatedStr](auto& n)
        {
            return "///      @li @b BitIdx_" + n + "." + getDeprecatedStr(n);
        });

    util::GenStringsList accessDoc;
    accessDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(accessDoc),
        [&getDeprecatedStr](auto& n)
        {
            return
                "///      @li @b getBitValue_" + n + "() and @b setBitValue_" +
                n + "()." + getDeprecatedStr(n);
        });

    util::GenStringsList bitsList;
    bitsList.reserve(bits.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsList),
        [&bits](auto& n)
        {
            auto iter = bits.find(n);
            if (iter == bits.end()) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                return strings::genEmptyString();
            }

            return n + "=" + std::to_string(iter->second.m_idx);
        });

    util::GenReplacementMap repl = {
        {"ACCESS_DOC", util::genStrListToString(accessDoc, "\n", "")},
        {"NAMES", util::genStrListToString(names, ",\n", "")},
        {"BITS_DOC", util::genStrListToString(bitsDoc, "\n", "")},
        {"BITS", util::genStrListToString(bitsList, ",\n", "")},
    };
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsSetField::commsDefBitNameFuncCodeInternal() const
{
    static const std::string Templ =
        "/// @brief Retrieve name of the bit.\n"
        "/// @see @ref #^#COMMON#$#::bitName().\n"
        "static const char* bitName(std::size_t idx)\n"
        "{\n"
        "    return #^#COMMON#$#::bitName(idx);\n"
        "}\n\n"
        "/// @brief Retrieve name of the bit.\n"
        "/// @see @ref #^#COMMON#$#::bitName().\n"
        "static const char* bitName(BitIdx idx)\n"
        "{\n"
        "    return bitName(static_cast<std::size_t>(idx));\n"
        "}\n";;

    util::GenReplacementMap repl = {
        {"COMMON", comms::genCommonScopeFor(*this, genGenerator())}
    };
    return util::genProcessTemplate(Templ, repl);
}

void CommsSetField::commsAddLengthOptInternal(commsdsl::gen::util::GenStringsList& opts) const
{
    auto bitLength = genParseObj().parseBitLength();
    if (bitLength != 0U) {
        opts.push_back("comms::option::def::FixedBitLength<" + util::genNumToString(bitLength) + '>');
        return;
    }

    opts.push_back("comms::option::def::FixedLength<" + util::genNumToString(genSetFieldParseObj().parseMinLength()) + '>');
}

void CommsSetField::commsAddDefaultValueOptInternal(commsdsl::gen::util::GenStringsList& opts) const
{
    auto obj = genSetFieldParseObj();
    std::uintmax_t defaultValue = 0U;
    if (obj.parseDefaultBitValue()) {
        defaultValue = ~defaultValue;
    }

    for (auto& bitInfo : obj.parseBits()) {
        if (CommsMaxBits <= bitInfo.second.m_idx) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        auto bitMask = static_cast<decltype(defaultValue)>(1U) << bitInfo.second.m_idx;
        if (bitInfo.second.m_defaultValue) {
            defaultValue |= bitMask;
        }
        else {
            defaultValue &= ~(bitMask);
        }
    }

    auto bitLength = obj.parseBitLength();
    if (bitLength == 0U) {
        bitLength = obj.parseMinLength() * 8U;
    }

    auto mask = ~static_cast<std::uintmax_t>(0);
    if (bitLength < CommsMaxBits) {
        mask = (static_cast<decltype(defaultValue)>(1U) << bitLength) - 1;
    }

    defaultValue &= mask;

    if (defaultValue == 0U) {
        return;
    }

    auto type = obj.parseType();
    if ((type == commsdsl::parse::ParseSetField::ParseType::Uint64) || (type == commsdsl::parse::ParseSetField::ParseType::Uintvar)) {
        auto str =
            "comms::option::def::DefaultBigUnsignedNumValue<" +
            util::genNumToString(static_cast<std::uintmax_t>(defaultValue), true) +
            '>';
        opts.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::def::DefaultNumValue<" +
        util::genNumToString(defaultValue, true) +
        '>';
    opts.push_back(std::move(str));
}

void CommsSetField::commsAddReservedBitsOptInternal(commsdsl::gen::util::GenStringsList& opts) const
{
    auto obj = genSetFieldParseObj();
    auto& gen = genGenerator();

    bool validCheckVersion =
        gen.genSchemaOf(*this).genVersionDependentCode() &&
        obj.parseValidCheckVersion();

    std::uintmax_t reservedMask = ~static_cast<std::uintmax_t>(0U);
    std::uintmax_t reservedValue = 0U;
    if (obj.parseReservedBitValue()) {
        reservedValue = ~reservedValue;
    }

    bool mustHandleBitsInValidFunc = false;

    for (auto& bitInfo : obj.parseBits()) {
        if (CommsMaxBits <= bitInfo.second.m_idx) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            continue;
        }

        if (validCheckVersion &&
            (!gen.genDoesElementExist(bitInfo.second.m_sinceVersion, bitInfo.second.m_deprecatedSince, true))) {
            continue;
        }

        auto bitMask = static_cast<decltype(reservedValue)>(1U) << bitInfo.second.m_idx;
        if (validCheckVersion &&
            ((obj.parseSinceVersion() < bitInfo.second.m_sinceVersion) || (bitInfo.second.m_deprecatedSince < obj.parseDeprecatedSince()))) {
            mustHandleBitsInValidFunc = true;
            reservedMask &= ~(bitMask);
            reservedValue &= ~(bitMask);
            continue;
        }

        if (!bitInfo.second.m_reserved) {
            reservedMask &= ~(bitMask);
            reservedValue &= ~(bitMask);
            continue;
        }

        if (bitInfo.second.m_reservedValue) {
            reservedValue |= bitMask;
        }
        else {
            reservedValue &= ~(bitMask);
        }
    }

    reservedValue &= reservedMask;

    auto bitLength = obj.parseBitLength();
    if (bitLength == 0U) {
        bitLength = obj.parseMinLength() * 8U;
    }

    auto mask = ~static_cast<std::uintmax_t>(0);
    if (bitLength < CommsMaxBits) {
        mask = (static_cast<decltype(reservedValue)>(1U) << bitLength) - 1;
    }

    reservedValue &= mask;
    reservedMask &= mask;

    if (mustHandleBitsInValidFunc) {
        opts.push_back("comms::option::def::VersionStorage");
    }

    if (reservedMask == 0U) {
        return;
    }

    std::string str =
        "comms::option::def::BitmaskReservedBits<" +
        util::genNumToString(reservedMask, true) +
        ", " +
        util::genNumToString(reservedValue, true) +
        '>';
    opts.push_back(std::move(str));
}

void CommsSetField::commsAddAvailableLengthLimitOptInternal(GenStringsList& opts) const
{
    if (genSetFieldParseObj().parseAvailableLengthLimit()) {
        util::genAddToStrList("comms::option::def::AvailableLengthLimit", opts);
    }
}

} // namespace commsdsl2comms
