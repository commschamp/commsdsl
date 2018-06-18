#include "SetField.h"

#include <type_traits>
#include <algorithm>
#include <iterator>

#include "Generator.h"
#include "common.h"
#include "IntField.h"

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::BitmaskValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::BitmaskValue<\n"
    "            #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#BITS_ACCESS#$#\n"
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
    "    comms::field::BitmaskValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#BITS_ACCESS#$#\n"
    "    #^#NAME#$#\n"
    "};\n"
);

const std::size_t MaxRangesInOpts = 5U;
const auto MaxBits = std::numeric_limits<std::uintmax_t>::digits;

bool shouldUseStruct(const common::ReplacementMap& replacements)
{
    auto hasNoValue =
        [&replacements](const std::string& val)
        {
            auto iter = replacements.find(val);
            return (iter == replacements.end()) || iter->second.empty();
        };

    return
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH");
}

} // namespace

void SetField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/BitmaskValue.h",
    };

    common::mergeIncludes(List, includes);
}

std::string SetField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(suffix)));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("BITS_ACCESS", getBitsAccess()));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string SetField::getFieldBaseParams() const
{
    auto obj = setFieldDslObj();
    auto endian = obj.endian();
    auto schemaEndian = generator().schemaEndian();
    assert(endian < commsdsl::Endian_NumOfValues);
    assert(schemaEndian < commsdsl::Endian_NumOfValues);

    if (schemaEndian == endian) {
        return common::emptyString();
    }

    return common::dslEndianToOpt(endian);
}

std::string SetField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    checkLengthOpt(options);
    checkDefaultValueOpt(options);
    checkReservedBitsOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string SetField::getBitsAccess() const
{
    auto obj = setFieldDslObj();
    std::uintmax_t usedBits = 0U;
    common::StringsList names;
    for (auto& bitInfo : obj.revBits()) {
        auto idx = bitInfo.first;
        if (MaxBits <= idx) {
            assert(!"Should not happen");
            continue;
        }

        auto mask = static_cast<std::uintmax_t>(1) << idx;
        usedBits |= mask;
        names.push_back(bitInfo.second);
    }

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

        common::StringsList accessDoc;
        accessDoc.reserve(names.size());
        std::transform(
            names.begin(), names.end(), std::back_inserter(accessDoc),
            [](auto& n)
            {
                return "///      @li @b FieldIdx_" + n + ", @b getBitValue_" + n + "() and @b setBitValue_" + n + "().";
            });

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("ACCESS_DOC", common::listToString(accessDoc, "\n", common::emptyString())));
        replacements.insert(std::make_pair("NAMES", common::listToString(names, ",\n", common::emptyString())));
        return common::processTemplate(Templ, replacements);
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

    common::StringsList bitsDoc;
    bitsDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsDoc),
        [](auto& n)
        {
            return "///      @li @b BitIdx_" + n + ".";
        });

    common::StringsList accessDoc;
    accessDoc.reserve(names.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(accessDoc),
        [](auto& n)
        {
            return "///      @li @b getBitValue_" + n + "() and @b setBitValue_" + n + "().";
        });

    auto bits = obj.bits();
    common::StringsList bitsList;
    bitsList.reserve(bits.size());
    std::transform(
        names.begin(), names.end(), std::back_inserter(bitsList),
        [&bits](auto& n)
        {
            auto iter = bits.find(n);
            if (iter == bits.end()) {
                assert(!"Should not happen");
                return common::emptyString();
            }

            return n + "=" + std::to_string(iter->second.m_idx);
        });

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ACCESS_DOC", common::listToString(accessDoc, "\n", common::emptyString())));
    replacements.insert(std::make_pair("NAMES", common::listToString(names, ",\n", common::emptyString())));
    replacements.insert(std::make_pair("BITS_DOC", common::listToString(bitsDoc, "\n", common::emptyString())));
    replacements.insert(std::make_pair("BITS", common::listToString(bitsList, ",\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string SetField::getValid() const
{
    auto custom = getCustomValid();
    if (!custom.empty()) {
        return custom;
    }

    auto obj = setFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        return common::emptyString();
    }

    using Key = std::tuple<unsigned, unsigned>;
    struct Value
    {
        std::uintmax_t m_reservedMask = 0U;
        std::uintmax_t m_reservedValue = 0U;
    };

    using Map = std::map<Key, Value>;
    Map bitsToCheck;

    auto& bits = obj.bits();
    for (auto& b : bits) {
        if ((b.second.m_sinceVersion == 0) &&
            (commsdsl::Protocol::notYetDeprecated() <= b.second.m_deprecatedSince)) {
            continue;
        }

        if (!generator().doesElementExist(b.second.m_sinceVersion, b.second.m_deprecatedSince, false)) {
            continue; // already handled
        }

//        if (!b.second.m_reserved) {
//            generator().logger().error("\tnot reserved");
//            continue;
//        }

        auto key = std::make_tuple(b.second.m_sinceVersion, b.second.m_deprecatedSince);
        auto& elem = bitsToCheck[key];

        auto bitMask = static_cast<std::uintmax_t>(1U) << b.second.m_idx;
        elem.m_reservedMask |= bitMask;

        if (b.second.m_reservedValue) {
            elem.m_reservedValue |= bitMask;
        }
        else {
            elem.m_reservedValue &= ~bitMask;
        }
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


    common::StringsList conditions;
    for (auto& info : bitsToCheck) {
        if (info.second.m_reservedMask == 0U) {
            continue;
        }

        auto* condTempl = &VersionBothCondTempl;
        if (std::get<0>(info.first) == 0U) {
            assert(std::get<1>(info.first) != commsdsl::Protocol::notYetDeprecated());
            condTempl = &VersionUntilCondTempl;
        }
        else if (commsdsl::Protocol::notYetDeprecated() <= std::get<1>(info.first)) {
            condTempl = &VersionFromCondTempl;
        }


        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("BITS_MASK", common::numToString(info.second.m_reservedMask, true)));
        replacements.insert(std::make_pair("VALUE_MASK", common::numToString(info.second.m_reservedValue, true)));
        replacements.insert(std::make_pair("FROM_VERSION", common::numToString(std::get<0>(info.first))));
        replacements.insert(std::make_pair("UNTIL_VERSION", common::numToString(std::get<1>(info.first))));
        conditions.push_back(common::processTemplate(*condTempl, replacements));
    }

    if (conditions.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Validity check function.\n"
        "bool valid() const\n"
        "{\n"
        "    if (!Base::valid()) {\n"
        "        return false;\n"
        "    }\n\n"
        "    #^#CONDITIONS#$#\n"
        "    return true;\n"
        "}\n";

    std::string condStr = common::listToString(conditions, "\n", common::emptyString());
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CONDITIONS", std::move(condStr)));
    return common::processTemplate(Templ, replacements);
}

void SetField::checkLengthOpt(SetField::StringsList& list) const
{
    auto bitLength = dslObj().bitLength();
    if (bitLength != 0U) {
        list.push_back("comms::option::FixedBitLength<" + common::numToString(bitLength) + '>');
        return;
    }

    list.push_back("comms::option::FixedLength<" + common::numToString(setFieldDslObj().minLength()) + '>');
}

void SetField::checkDefaultValueOpt(StringsList& list) const
{
    auto obj = setFieldDslObj();
    std::uintmax_t defaultValue = 0U;
    if (obj.defaultBitValue()) {
        defaultValue = ~defaultValue;
    }

    for (auto& bitInfo : obj.bits()) {
        if (MaxBits <= bitInfo.second.m_idx) {
            assert(!"Should not happen");
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
    if ((type == commsdsl::SetField::Type::Uint64) ||
        (type == commsdsl::SetField::Type::Uintvar)) {
        auto str =
            "comms::option::DefaultBigUnsignedNumValue<" +
            common::numToString(static_cast<std::uintmax_t>(defaultValue), true) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::DefaultNumValue<" +
        common::numToString(defaultValue, true) +
        '>';
    list.push_back(std::move(str));
}

void SetField::checkReservedBitsOpt(SetField::StringsList& list) const
{
    auto obj = setFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    std::uintmax_t reservedMask = ~static_cast<std::uintmax_t>(0U);
    std::uintmax_t reservedValue = 0U;
    if (obj.reservedBitValue()) {
        reservedValue = ~reservedValue;
    }

    bool mustHandleBitsInValidFunc = false;

    for (auto& bitInfo : obj.bits()) {
        if (MaxBits <= bitInfo.second.m_idx) {
            assert(!"Should not happen");
            continue;
        }

        if (validCheckVersion &&
            (!generator().doesElementExist(bitInfo.second.m_sinceVersion, bitInfo.second.m_deprecatedSince, true))) {
            continue;
        }

        auto bitMask = static_cast<decltype(reservedValue)>(1U) << bitInfo.second.m_idx;
        if (validCheckVersion &&
            ((bitInfo.second.m_sinceVersion != 0U) || (bitInfo.second.m_deprecatedSince < commsdsl::Protocol::notYetDeprecated()))) {
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
        list.push_back("comms::option::VersionStorage");
    }

    if (reservedMask == 0U) {
        return;
    }

    std::string str =
        "comms::option::BitmaskReservedBits<" +
        common::numToString(reservedMask, true) +
        ", " +
        common::numToString(reservedValue, true) +
        '>';
    list.push_back(std::move(str));
}


} // namespace commsdsl2comms
