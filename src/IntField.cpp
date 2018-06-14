#include "IntField.h"

#include <type_traits>

#include "Generator.h"
#include "common.h"

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::IntValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::IntValue<\n"
    "            #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#FIELD_TYPE#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
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
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::IntValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#FIELD_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    #^#NAME#$#\n"
    "};\n"
);

bool shouldUseStruct(const common::ReplacementMap& replacements)
{
    auto hasNoValue =
        [&replacements](const std::string& val)
        {
            auto iter = replacements.find(val);
            return (iter == replacements.end()) || iter->second.empty();
        };

    return
        hasNoValue("SPECIALS") &&
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH");
}

} // namespace

const Field::IncludesList& IntField::extraIncludesImpl() const
{
    static const IncludesList List = {
        "comms/field/IntValue.h",
        "<cstdint>"
    };
    return List;
}

std::string IntField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
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

std::string IntField::getFieldBaseParams() const
{
    auto obj = intFieldDslObj();
    auto endian = obj.endian();
    auto schemaEndian = generator().schemaEndian();
    assert(endian < commsdsl::Endian_NumOfValues);
    assert(schemaEndian < commsdsl::Endian_NumOfValues);

    if (schemaEndian == endian) {
        return common::emptyString();
    }

    return common::dslEndianToOpt(endian);
}

const std::string& IntField::getFieldType() const
{
    static const std::string TypeMap[] = {
        /* Int8 */ "std::int8_t",
        /* Uint8 */ "std::uint8_t",
        /* Int16 */ "std::int16_t",
        /* Uint16 */ "std::uint16_t",
        /* Int32 */ "std::int32_t",
        /* Uint32 */ "std::uint32_t",
        /* Int64 */ "std::int64_t",
        /* Uint64 */ "std::uint64_t",
        /* Intvar */ common::emptyString(),
        /* Uintvar */ common::emptyString()
    };

    static const std::size_t TypeMapSize = std::extent<decltype(TypeMap)>::value;
    static_assert(TypeMapSize == static_cast<decltype(TypeMapSize)>(commsdsl::IntField::Type::NumOfValues),
            "Incorrect map");

    auto obj = intFieldDslObj();
    std::size_t idx = static_cast<std::size_t>(obj.type());
    if (TypeMapSize <= idx) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    auto& typeStr = TypeMap[idx];
    if (!typeStr.empty()) {
        return typeStr;
    }

    // Variable length
    auto offset = idx - static_cast<decltype(idx)>(commsdsl::IntField::Type::Intvar);
    assert(offset < 2U);
    auto len = obj.maxLength();
    if (len <= 2U) {
        auto base = static_cast<decltype(idx)>(commsdsl::IntField::Type::Int16);
        return TypeMap[base + offset];
    }

    if (len <= 4U) {
        auto base = static_cast<decltype(idx)>(commsdsl::IntField::Type::Int32);
        return TypeMap[base + offset];
    }

    auto base = static_cast<decltype(idx)>(commsdsl::IntField::Type::Int64);
    return TypeMap[base + offset];
}

std::string IntField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    checkDefaultValueOpt(options);
    checkLengthOpt(options);
    checkSerOffsetOpt(options);
    checkScalingOpt(options);
    checkUnitsOpt(options);
    checkValidRangesOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string IntField::getSpecials() const
{
    auto obj = intFieldDslObj();
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
            "static constexpr typename Base::ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return static_cast<typename Base::ValueType>(#^#SPEC_VAL#$#);\n"
            "}\n\n"
            "/// @brief Check the value is equal to special <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "bool is#^#SPEC_ACC#$#() const\n"
            "{\n"
            "    return Base::value() == value#^#SPEC_ACC#$#();\n"
            "}\n\n"
            "/// @brief Assign special value <b>\"#^#SPEC_NAME#$#\"</b> to the field.\n"
            "void set#^#SPEC_ACC#$#()\n"
            "{\n"
            "    Base::value() = value#^#SPEC_ACC#$#();\n"
            "}\n"
        );

        std::string specVal;
        auto type = obj.type();
        if ((type == commsdsl::IntField::Type::Uint64) ||
            (type == commsdsl::IntField::Type::Uintvar)) {
            specVal = common::numToString(static_cast<std::uintmax_t>(s.second.m_value));
        }
        else {
            specVal = common::numToString(s.second.m_value);
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("SPEC_NAME", s.first));
        replacements.insert(std::make_pair("SPEC_ACC", common::nameToClassCopy(s.first)));
        replacements.insert(std::make_pair("SPEC_VAL", std::move(specVal)));

        result += common::processTemplate(Templ, replacements);
    }
    return result;
}

std::string IntField::getValid() const
{
    auto custom = getCustomValid();
    if (!custom.empty()) {
        return custom;
    }

    auto obj = intFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {
        return common::emptyString();
    }

    auto validRanges = obj.validRanges(); // copy
    validRanges.erase(
        std::remove_if(
            validRanges.begin(), validRanges.end(),
            [this](auto& r)
            {
                return !this->generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince);
            }),
        validRanges.end());

    if (validRanges.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Custom validity check.\n"
        "bool valid() const\n"
        "{\n"
        "    if (Base::valid()) {\n"
        "        return true;\n"
        "    }\n\n"
        "    #^#RANGES_CHECKS#$#\n"
        "    return false;\n"
        "}\n";

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::IntField::Type::Uint64) ||
        (type == commsdsl::IntField::Type::Uintvar);

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
            minVal = common::numToString(static_cast<std::uintmax_t>(r.m_min));
            maxVal = common::numToString(static_cast<std::uintmax_t>(r.m_max));
        }
        else {
            minVal = common::numToString(r.m_min);
            maxVal = common::numToString(r.m_max);
        }

        common::StringsList conds;
        if (0U < r.m_sinceVersion) {
            conds.push_back('(' + common::numToString(r.m_sinceVersion) + " <= Base::getVersion())");
        }

        if (r.m_deprecatedSince < commsdsl::Protocol::notYetDeprecated()) {
            conds.push_back("(Base::getVersion() < " + common::numToString(r.m_deprecatedSince) + ")");
        }

        if (r.m_min == r.m_max) {
            conds.push_back("(static_cast<typename Base::ValueType>(" + minVal + ") == Base::value())");
        }
        else {
            conds.push_back("(static_cast<typename Base::ValueType>(" + minVal + ") <= Base::value())");
            conds.push_back("(Base::value() <= static_cast<typename Base::ValueType>(" + maxVal + "))");
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("COND", common::listToString(conds, " &&\n", common::emptyString())));
        rangesChecks += common::processTemplate(RangeTempl, replacements);
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("RANGES_CHECKS", std::move(rangesChecks)));
    return common::processTemplate(Templ, replacements);
}

void IntField::checkDefaultValueOpt(StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto defaultValue = obj.defaultValue();
    if ((defaultValue == 0) &&
        (semanticType() == commsdsl::Field::SemanticType::Version)) {
        std::string opt = "comms::option::DefaultNumValue<";
        opt += common::numToString(generator().schemaVersion());
        opt += '>';
        list.push_back(std::move(opt));
        return;
    }

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.type();
    if ((type == commsdsl::IntField::Type::Uint64) ||
        (type == commsdsl::IntField::Type::Uintvar)) {
        auto str =
            "comms::option::DefaultBigUnsignedNumValue<" +
            common::numToString(static_cast<std::uintmax_t>(defaultValue)) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    auto str =
        "comms::option::DefaultNumValue<" +
        common::numToString(defaultValue) +
        '>';
    list.push_back(std::move(str));
}

void IntField::checkLengthOpt(StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto type = obj.type();
    if ((type == commsdsl::IntField::Type::Intvar) ||
        (type == commsdsl::IntField::Type::Uintvar)) {
        auto str =
            "comms::option::VarLength<" +
            common::numToString(obj.minLength()) +
            ", " +
            common::numToString(obj.maxLength()) +
            '>';
        list.push_back(std::move(str));
        return;
    }

    if (obj.bitLength() != 0U) {
        auto str =
            "comms::option::FixedBitLength<" +
            common::numToString(obj.bitLength()) +
            '>';
        list.push_back(std::move(str));
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
    static_assert(LengthMapSize == static_cast<decltype(LengthMapSize)>(commsdsl::IntField::Type::NumOfValues),
            "Incorrect map");

    std::size_t idx = static_cast<std::size_t>(type);
    if (LengthMapSize <= idx) {
        return;
    }

    assert(LengthMap[idx] != 0);
    if (LengthMap[idx] != obj.minLength()) {
        auto str =
            "comms::option::FixedLength<" +
            common::numToString(obj.minLength()) +
            '>';
        list.push_back(std::move(str));
    }
}

void IntField::checkSerOffsetOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto serOffset = obj.serOffset();
    if (serOffset == 0) {
        return;
    }

    auto str =
        "comms::option::NumValueSerOffset<" +
        common::numToString(serOffset) +
        '>';
    list.push_back(std::move(str));
}

void IntField::checkScalingOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto scaling = obj.scaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return;
    }

    if ((num == 0) || (denom == 0)) {
        assert(!"Should not happen");
        return;
    }

    auto str =
        "comms::option::ScalingRatio<" +
        common::numToString(num) +
        ", " +
        common::numToString(denom) +
        '>';
    list.push_back(std::move(str));
}

void IntField::checkUnitsOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto units = obj.units();

    if (units == commsdsl::Units::Unknown) {
        return;
    }

    if (commsdsl::Units::NumOfValues <= units) {
        assert(!"Should not happen");
        return;
    }

    static const std::string UnitsMap[] = {
        /* Unknown */ common::emptyString(),
        /* Nanoseconds */ "Nanoseconds",
        /* Microseconds */ "Microseconds",
        /* Milliseconds */ "Milliseconds",
        /* Seconds */ "Seconds",
        /* Minutes */ "Minutes",
        /* Hours */ "Hours",
        /* Days */ "Days",
        /* Weeks */ "Weeks",
        /* Nanometers */ "Nanometers",
        /* Micrometers */ "Micrometers",
        /* Millimeters */ "Millimeters",
        /* Centimeters */ "Centimeters",
        /* Meters */ "Meters",
        /* Kilometers */ "Kilometers",
        /* NanometersPerSecond */ "NanometersPerSecond",
        /* MicrometersPerSecond */ "MicrometersPerSecond",
        /* MillimetersPerSecond */ "MillimetersPerSecond",
        /* CentimetersPerSecond */ "CentimetersPerSecond",
        /* MetersPerSecond */ "MetersPerSecond",
        /* KilometersPerSecond */ "KilometersPerSecond",
        /* KilometersPerHour */ "KilometersPerHour",
        /* Hertz */ "Hertz",
        /* KiloHertz */ "Kilohertz",
        /* MegaHertz */ "Megahertz",
        /* GigaHertz */ "Gigahertz",
        /* Degrees */ "Degrees",
        /* Radians */ "Radians",
        /* Nanoamps */ "Nanoamps",
        /* Microamps */ "Microamps",
        /* Milliamps */ "Milliamps",
        /* Amps */ "Amps",
        /* Kiloamps */ "Kiloamps",
        /* Nanovolts */ "Nanovolts",
        /* Microvolts */ "Microvolts",
        /* Millivolts */ "Millivolts",
        /* Volts */ "Volts",
        /* Kilovolts */ "Kilovolts",
    };

    static const std::size_t UnitsMapSize =
        std::extent<decltype(UnitsMap)>::value;
    static_assert(static_cast<decltype(UnitsMapSize)>(commsdsl::Units::NumOfValues) == UnitsMapSize,
        "Invalid Map");

    auto idx = static_cast<unsigned>(units);
    list.push_back("comms::option::Units" + UnitsMap[idx]);
}

void IntField::checkValidRangesOpt(IntField::StringsList& list) const
{
    auto obj = intFieldDslObj();
    auto validRanges = obj.validRanges(); // copy
    if (validRanges.empty()) {
        return;
    }

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::IntField::Type::Uint64) ||
        ((type != commsdsl::IntField::Type::Uintvar) && (obj.maxLength() >= sizeof(std::int64_t)));

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
        if (!generator().doesElementExist(r.m_sinceVersion, r.m_deprecatedSince, true)) {
            continue;
        }

        if (validCheckVersion && (generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince))) {
            versionStorageRequired = true;
            continue;
        }

        bool big = false;
        std::string str = "comms::option::";
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
                str += common::numToString(static_cast<std::uintmax_t>(r.m_min));
                str += '>';
            }
            else {
                str += "ValidBigUnsignedNumValueRange<";
                str += common::numToString(static_cast<std::uintmax_t>(r.m_min));
                str += ", ";
                str += common::numToString(static_cast<std::uintmax_t>(r.m_max));
                str += '>';
            }

            list.push_back(std::move(str));
            big = true;
            addedRangeOpt = true;
        } while (false);

        if (big) {
            continue;
        }

        if (r.m_min == r.m_max) {
            str += "ValidNumValue<";
            str += common::numToString(r.m_min);
            str += '>';
        }
        else {
            str += "ValidNumValueRange<";
            str += common::numToString(r.m_min);
            str += ", ";
            str += common::numToString(r.m_max);
            str += '>';
        }

        list.push_back(std::move(str));
        addedRangeOpt = true;
    }

    if (versionStorageRequired) {
        list.push_back("comms::option::VersionStorage");

        if (!addedRangeOpt) {
            list.push_back("comms::option::InvalidByDefault");
        }
    }
}

} // namespace commsdsl2comms
