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

namespace commsdsl2new
{

namespace 
{

const auto MaxBits = std::numeric_limits<std::uintmax_t>::digits;

} // namespace 
    

CommsSetField::CommsSetField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsSetField::prepareImpl()
{
    return 
        Base::prepareImpl() && 
        commsPrepare();
}

bool CommsSetField::writeImpl() const
{
    return commsWrite();
}

CommsSetField::IncludesList CommsSetField::commsCommonIncludesImpl() const
{
    IncludesList result = {
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

    util::ReplacementMap repl = {
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"BIT_NAME_FUNC", commsCommonBitNameFuncCodeInternal()},
    };

    return util::processTemplate(Templ, repl);
}

CommsSetField::IncludesList CommsSetField::commsDefIncludesImpl() const
{
    IncludesList result = {
        "comms/field/BitmaskValue.h"
    };

    return result;
}

std::string CommsSetField::commsBaseClassDefImpl() const
{
    static const std::string Templ = 
        "comms::field::BitmaskValue<\n"
        "    #^#PROT_NAMESPACE#$#::field::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
        "    #^#FIELD_OPTS#$#\n"
        ">";

    auto& gen = generator();
    auto dslObj = setDslObj();
    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", gen.mainNamespace()},
        {"FIELD_BASE_PARAMS", commsFieldBaseParams(dslObj.endian())},
        {"FIELD_OPTS", commsDefFieldOptsInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsSetField::commsDefPublicCodeImpl() const
{
    static const std::string Templ = 
        "#^#BITS_ACCESS#$#\n"
        "#^#BIT_NAME#$#";

    util::ReplacementMap repl = {
        {"BITS_ACCESS", commsDefBitsAccessCodeInternal()},
        {"BIT_NAME", commsDefBitNameFuncCodeInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsSetField::commsDefValidFuncBodyImpl() const
{
    auto obj = setDslObj();
    auto& gen = generator();

    bool validCheckVersion =
        gen.versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        return strings::emptyString();
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
    auto& bits = obj.bits();
    for (auto& b : bits) {
        if ((b.second.m_sinceVersion <= obj.sinceVersion()) &&
            (obj.deprecatedSince() <= b.second.m_deprecatedSince)) {
            continue;
        }

        if (!gen.doesElementExist(b.second.m_sinceVersion, b.second.m_deprecatedSince, false)) {
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

    util::StringsList conditions;
    for (auto& info : bitsToCheck) {
        if (info.second.m_reservedMask == 0U) {
            continue;
        }

            static const std::string VersionBothCondTempl =
                "if (((Base::getVersion() < #^#FROM_VERSION#$#) || (#^#UNTIL_VERSION#$# <= Base::getVersion())) && \n"
                "    ((Base::value() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

            static const std::string VersionFromCondTempl =
                "if ((Base::getVersion() < #^#FROM_VERSION#$#) &&\n"
                "    ((Base::value() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

            static const std::string VersionUntilCondTempl =
                "if ((#^#UNTIL_VERSION#$# <= Base::getVersion()) &&\n"
                "    ((Base::value() & #^#BITS_MASK#$#) != #^#VALUE_MASK#$#)) {\n"
                "    return false;\n"
                "}\n";

        auto* condTempl = &VersionBothCondTempl;
        if (std::get<0>(info.first) == 0U) {
            assert(std::get<1>(info.first) != commsdsl::parse::Protocol::notYetDeprecated());
            condTempl = &VersionUntilCondTempl;
        }
        else if (commsdsl::parse::Protocol::notYetDeprecated() <= std::get<1>(info.first)) {
            condTempl = &VersionFromCondTempl;
        }


        util::ReplacementMap repl = {
            {"BITS_MASK", util::numToString(info.second.m_reservedMask, true)},
            {"VALUE_MASK", util::numToString(info.second.m_reservedValue, true)},
            {"FROM_VERSION", util::numToString(std::get<0>(info.first))},
            {"UNTIL_VERSION", util::numToString(std::get<1>(info.first))},
        };
        conditions.push_back(util::processTemplate(*condTempl, repl));
    }

    for (auto idx : repeatingBits) {
        auto elems = obj.revBits().equal_range(idx);
        assert(elems.first != elems.second);
        std::vector<std::pair<unsigned, unsigned> > versionRanges;
        for (auto iter = elems.first; iter != elems.second; ++iter) {
            auto& bitName = iter->second;
            auto bitIter = bits.find(bitName);
            if (bitIter == bits.end()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
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

        if (prev < obj.deprecatedSince()) {
            versionRanges.push_back(std::make_pair(prev, obj.deprecatedSince()));
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

        util::StringsList extraConds;
        for (auto& r : versionRanges) {
            if ((r.second == 0U) || (r.first == r.second)) {
                continue; // ignore invalid ranges
            }

            static const std::string Templ =
                "if ((#^#FROM_VERSION#$# <= Base::getVersion()) &&\n"
                "    (Base::getVersion() < #^#UNTIL_VERSION#$#)) {\n"
                "    return false;\n"
                "}";

            util::ReplacementMap repl = {
                {"FROM_VERSION", util::numToString(r.first)},
                {"UNTIL_VERSION", util::numToString(r.second)},
            };

            extraConds.push_back(util::processTemplate(Templ, repl));
        }

        static const std::string Templ =
            "if ((Base::value() & #^#RESERVED_MASK#$#) != #^#RESERVED_VALUE#$#) {\n"
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
            util::ReplacementMap repl = {
                {"RESERVED_MASK", util::numToString(bitMask, true)},
                {"RESERVED_VALUE", util::numToString(bitValue, true)},
                {"CONDITIONS", util::strListToString(extraConds, "\n\n", "")}
            };

            conditions.push_back(util::processTemplate(Templ, repl));
        }
    }

    if (conditions.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "if (!Base::valid()) {\n"
        "    return false;\n"
        "}\n\n"
        "#^#CONDITIONS#$#\n"
        "return true;\n"
        ;

    util::ReplacementMap repl = {
        {"CONDITIONS", util::strListToString(conditions, "\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsSetField::commsCommonBitNameFuncCodeInternal() const
{
    auto obj = setDslObj();
    auto& bits = obj.bits();
    auto& revBits = obj.revBits();
    std::intmax_t nextBit = 0;
    StringsList names;
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
                    return strings::emptyString();
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
        if ((!obj.isNonUniqueAllowed()) || 
            (generator().schemaVersion() < bitIter->second.m_deprecatedSince) ||
            (obj.isUnique())) {
            addElementNameFunc(*bitIter);
            continue;
        }

        auto allRevBits = revBits.equal_range(b.first);
        bool foundNotDeprecated = false;
        for (auto iter = allRevBits.first; iter != allRevBits.second; ++iter) {
            auto bIter = bits.find(iter->second);
            assert(bIter != bits.end());
            if (generator().schemaVersion() < bIter->second.m_deprecatedSince) {
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

    static const std::string Templ =
        "/// @brief Retrieve name of the bit of\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "static const char* bitName(std::size_t idx)\n"
        "{\n"
        "    static const char* Map[] = {\n"
        "        #^#NAMES#$#\n"
        "    };\n\n"
        "    static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n"
        "    if (MapSize <= idx) {\n"
        "        return nullptr;\n"
        "    }\n\n"
        "    return Map[idx];\n"
        "}\n";

    util::ReplacementMap repl = {
        {"NAMES", util::strListToString(names, ",\n", "")},
        {"SCOPE", comms::scopeFor(*this, generator())}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsSetField::commsDefFieldOptsInternal() const
{
    util::StringsList opts;

    commsAddFieldDefOptions(opts);
    commsAddLengthOptInternal(opts);
    commsAddDefaultValueOptInternal(opts);
    commsAddReservedBitsOptInternal(opts);

    return util::strListToString(opts, ",\n", "");
}

std::string CommsSetField::commsDefBitsAccessCodeInternal() const
{
    auto obj = setDslObj();
    std::uintmax_t usedBits = 0U;
    util::StringsList names;

    std::map<std::string, unsigned> deprecatedBits;
    auto bits = obj.bits();
    for (auto& bitInfo : obj.revBits()) {
        auto idx = bitInfo.first;
        if (MaxBits <= idx) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            continue;
        }

        auto mask = static_cast<std::uintmax_t>(1) << idx;
        usedBits |= mask;
        names.push_back(bitInfo.second);

        auto iter = bits.find(bitInfo.second);
        assert(iter != bits.end());
        if (generator().isElementDeprecated(iter->second.m_deprecatedSince)) {
            deprecatedBits.insert(std::make_pair(bitInfo.second, iter->second.m_deprecatedSince));
        }
    }

    auto getDeprecatedStr = 
        [&deprecatedBits](const std::string& n) -> std::string
        {
            auto iter = deprecatedBits.find(n);
            if (iter == deprecatedBits.end()) {
                return strings::emptyString();
            }

            return 
                "\n"
                "///          Deprecated since version " + std::to_string(iter->second) + '.';
        };    

    if (obj.isUnique() && (((usedBits + 1) & usedBits) == 0U)) {
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

        util::StringsList accessDoc;
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

        util::ReplacementMap repl = {
            {"ACCESS_DOC", util::strListToString(accessDoc, "\n", "")},
            {"NAMES", util::strListToString(names, ",\n", "")}
        };
        return util::processTemplate(Templ, repl);
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

    util::StringsList bitsDoc;
    bitsDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsDoc),
        [&getDeprecatedStr](auto& n)
        {
            return "///      @li @b BitIdx_" + n + "." + getDeprecatedStr(n);
        });

    util::StringsList accessDoc;
    accessDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(accessDoc),
        [&getDeprecatedStr](auto& n)
        {
            return 
                "///      @li @b getBitValue_" + n + "() and @b setBitValue_" + 
                n + "()." + getDeprecatedStr(n);
        });

    util::StringsList bitsList;
    bitsList.reserve(bits.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsList),
        [&bits](auto& n)
        {
            auto iter = bits.find(n);
            if (iter == bits.end()) {
                static constexpr bool Should_not_happen = false;
                static_cast<void>(Should_not_happen);
                assert(Should_not_happen);
                return strings::emptyString();
            }

            return n + "=" + std::to_string(iter->second.m_idx);
        });

    util::ReplacementMap repl = {
        {"ACCESS_DOC", util::strListToString(accessDoc, "\n", "")},
        {"NAMES", util::strListToString(names, ",\n", "")},
        {"BITS_DOC", util::strListToString(bitsDoc, "\n", "")},
        {"BITS", util::strListToString(bitsList, ",\n", "")},
    };
    return util::processTemplate(Templ, repl);    
}

std::string CommsSetField::commsDefBitNameFuncCodeInternal() const
{
    static const std::string Templ =
        "/// @brief Retrieve name of the bit.\n"
        "/// @see @ref #^#COMMON#$#::bitName().\n"
        "static const char* bitName(BitIdx idx)\n"
        "{\n"
        "    return\n"
        "        #^#COMMON#$#::bitName(\n"
        "            static_cast<std::size_t>(idx));\n"
        "}\n";

    util::ReplacementMap repl = {
        {"COMMON", comms::commonScopeFor(*this, generator())}
    };
    return util::processTemplate(Templ, repl);
}

void CommsSetField::commsAddLengthOptInternal(commsdsl::gen::util::StringsList& opts) const
{
    auto bitLength = dslObj().bitLength();
    if (bitLength != 0U) {
        opts.push_back("comms::option::def::FixedBitLength<" + util::numToString(bitLength) + '>');
        return;
    }

    opts.push_back("comms::option::def::FixedLength<" + util::numToString(setDslObj().minLength()) + '>');
}

void CommsSetField::commsAddDefaultValueOptInternal(commsdsl::gen::util::StringsList& opts) const
{
    auto obj = setDslObj();
    std::uintmax_t defaultValue = 0U;
    if (obj.defaultBitValue()) {
        defaultValue = ~defaultValue;
    }

    for (auto& bitInfo : obj.bits()) {
        if (MaxBits <= bitInfo.second.m_idx) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
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

    auto bitLength = obj.bitLength();
    if (bitLength == 0U) {
        bitLength = obj.minLength() * 8U;
    }

    auto mask = ~static_cast<std::uintmax_t>(0);
    if (bitLength < MaxBits) {
        mask = (static_cast<decltype(defaultValue)>(1U) << bitLength) - 1;
    }

    defaultValue &= mask;

    if (defaultValue == 0U) {
        return;
    }

    auto type = obj.type();
    if ((type == commsdsl::parse::SetField::Type::Uint64) || (type == commsdsl::parse::SetField::Type::Uintvar)) {
        auto str =
            "comms::option::def::DefaultBigUnsignedNumValue<" +
            util::numToString(static_cast<std::uintmax_t>(defaultValue), true) +
            '>';
        opts.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::def::DefaultNumValue<" +
        util::numToString(defaultValue, true) +
        '>';
    opts.push_back(std::move(str));
}

void CommsSetField::commsAddReservedBitsOptInternal(commsdsl::gen::util::StringsList& opts) const
{
    auto obj = setDslObj();
    auto& gen = generator();

    bool validCheckVersion =
        gen.versionDependentCode() &&
        obj.validCheckVersion();

    std::uintmax_t reservedMask = ~static_cast<std::uintmax_t>(0U);
    std::uintmax_t reservedValue = 0U;
    if (obj.reservedBitValue()) {
        reservedValue = ~reservedValue;
    }

    bool mustHandleBitsInValidFunc = false;

    for (auto& bitInfo : obj.bits()) {
        if (MaxBits <= bitInfo.second.m_idx) {
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            continue;
        }

        if (validCheckVersion &&
            (!gen.doesElementExist(bitInfo.second.m_sinceVersion, bitInfo.second.m_deprecatedSince, true))) {
            continue;
        }

        auto bitMask = static_cast<decltype(reservedValue)>(1U) << bitInfo.second.m_idx;
        if (validCheckVersion &&
            ((obj.sinceVersion() < bitInfo.second.m_sinceVersion) || (bitInfo.second.m_deprecatedSince < obj.deprecatedSince()))) {
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

    auto bitLength = obj.bitLength();
    if (bitLength == 0U) {
        bitLength = obj.minLength() * 8U;
    }

    auto mask = ~static_cast<std::uintmax_t>(0);
    if (bitLength < MaxBits) {
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
        util::numToString(reservedMask, true) +
        ", " +
        util::numToString(reservedValue, true) +
        '>';
    opts.push_back(std::move(str));
}

} // namespace commsdsl2new
