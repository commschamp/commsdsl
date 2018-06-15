#include "EnumField.h"

#include <type_traits>

#include "Generator.h"
#include "common.h"
#include "IntField.h"

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#ENUMERATION#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::EnumValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#ENUM_TYPE#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::EnumValue<\n"
    "            #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#ENUM_TYPE#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "};\n"
);

const std::string StructTemplate(
    "#^#ENUMERATION#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::EnumValue<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "        #^#ENUM_TYPE#$#\n"
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
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH");
}

} // namespace

common::StringsList EnumField::getValuesList(bool description) const
{
    auto obj = enumFieldDslObj();
    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::EnumField::Type::Uint64) ||
        (type == commsdsl::EnumField::Type::Uintvar);

    common::StringsList values;
    for (auto& v : obj.revValues()) {
        std::string str = v.second + " = ";

        if (bigUnsigned) {
            str += common::numToString(static_cast<std::uintmax_t>(v.first));
        }
        else {
            str += common::numToString(v.first);
        }

        if (description) {
            // TODO: proper description when implemented
            str += ", ///< value @b " + v.second;
        }

        values.push_back(std::move(str));
    }
    return values;
}

std::string EnumField::getValuesDefinition() const
{
    common::StringsList values = getValuesList();

    return common::listToString(values, "\n", common::emptyString());
}

std::string EnumField::getValueName(std::intmax_t value) const
{
    auto obj = enumFieldDslObj();
    auto& revValues = obj.revValues();
    auto iter = revValues.find(value);
    if (iter != revValues.end()) {
        return iter->second;
    }

    return common::emptyString();
}

void EnumField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/EnumValue.h",
        "<cstdint>"
    };

    common::mergeIncludes(List, includes);
    if (dslObj().semanticType() == commsdsl::Field::SemanticType::MessageId) {
        auto inc =
            generator().mainNamespace() + '/' +
            common::msgIdEnumNameStr() + common::headerSuffix();
        common::mergeInclude(inc, includes);
    }
}

std::string EnumField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ENUMERATION", getEnumeration()));
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(suffix)));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("ENUM_TYPE", getEnumType(suffix)));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["ENUM_TYPE"] += ',';
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string EnumField::getEnumeration() const
{
    if (dslObj().semanticType() == commsdsl::Field::SemanticType::MessageId) {
        return common::emptyString();
    }

    static const std::string Templ =
        "enum class #^#NAME#$#Val : #^#TYPE#$#\n"
        "{\n"
        "    #^#VALUES#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("TYPE", IntField::convertType(enumFieldDslObj().type())));
    replacements.insert(std::make_pair("VALUES", getValuesDefinition()));
    return common::processTemplate(Templ, replacements);
}

std::string EnumField::getFieldBaseParams() const
{
    auto obj = enumFieldDslObj();
    auto endian = obj.endian();
    auto schemaEndian = generator().schemaEndian();
    assert(endian < commsdsl::Endian_NumOfValues);
    assert(schemaEndian < commsdsl::Endian_NumOfValues);

    if (schemaEndian == endian) {
        return common::emptyString();
    }

    return common::dslEndianToOpt(endian);
}

std::string EnumField::getEnumType(const std::string& suffix) const
{
    if (dslObj().semanticType() == commsdsl::Field::SemanticType::MessageId) {
        return generator().mainNamespace() + "::" + common::msgIdEnumNameStr();
    }

    return common::nameToClassCopy(dslObj().name()) + suffix + "Val";
}

std::string EnumField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    checkDefaultValueOpt(options);
    checkValidRangesOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string EnumField::getValid() const
{
//    auto custom = getCustomValid();
//    if (!custom.empty()) {
//        return custom;
//    }

//    auto obj = enumFieldDslObj();

//    bool validCheckVersion =
//        generator().versionDependentCode() &&
//        obj.validCheckVersion();

//    if (!validCheckVersion) {
//        return common::emptyString();
//    }

//    auto validRanges = obj.validRanges(); // copy
//    validRanges.erase(
//        std::remove_if(
//            validRanges.begin(), validRanges.end(),
//            [this](auto& r)
//            {
//                return !this->generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince);
//            }),
//        validRanges.end());

//    if (validRanges.empty()) {
//        return common::emptyString();
//    }

//    static const std::string Templ =
//        "/// @brief Custom validity check.\n"
//        "bool valid() const\n"
//        "{\n"
//        "    if (Base::valid()) {\n"
//        "        return true;\n"
//        "    }\n\n"
//        "    #^#RANGES_CHECKS#$#\n"
//        "    return false;\n"
//        "}\n";

//    auto type = obj.type();
//    bool bigUnsigned =
//        (type == commsdsl::EnumField::Type::Uint64) ||
//        (type == commsdsl::EnumField::Type::Uintvar);

//    std::string rangesChecks;
//    for (auto& r : validRanges) {
//        if (!rangesChecks.empty()) {
//            rangesChecks += '\n';
//        }

//        static const std::string RangeTempl =
//            "if (#^#COND#$#) {\n"
//            "    return true;\n"
//            "}\n";

//        std::string minVal;
//        std::string maxVal;

//        if (bigUnsigned) {
//            minVal = common::numToString(static_cast<std::uintmax_t>(r.m_min));
//            maxVal = common::numToString(static_cast<std::uintmax_t>(r.m_max));
//        }
//        else {
//            minVal = common::numToString(r.m_min);
//            maxVal = common::numToString(r.m_max);
//        }

//        common::StringsList conds;
//        if (0U < r.m_sinceVersion) {
//            conds.push_back('(' + common::numToString(r.m_sinceVersion) + " <= Base::getVersion())");
//        }

//        if (r.m_deprecatedSince < commsdsl::Protocol::notYetDeprecated()) {
//            conds.push_back("(Base::getVersion() < " + common::numToString(r.m_deprecatedSince) + ")");
//        }

//        if (r.m_min == r.m_max) {
//            conds.push_back("(static_cast<typename Base::ValueType>(" + minVal + ") == Base::value())");
//        }
//        else {
//            conds.push_back("(static_cast<typename Base::ValueType>(" + minVal + ") <= Base::value())");
//            conds.push_back("(Base::value() <= static_cast<typename Base::ValueType>(" + maxVal + "))");
//        }

//        common::ReplacementMap replacements;
//        replacements.insert(std::make_pair("COND", common::listToString(conds, " &&\n", common::emptyString())));
//        rangesChecks += common::processTemplate(RangeTempl, replacements);
//    }

//    common::ReplacementMap replacements;
//    replacements.insert(std::make_pair("RANGES_CHECKS", std::move(rangesChecks)));
//    return common::processTemplate(Templ, replacements);
    return common::emptyString();
}

void EnumField::checkDefaultValueOpt(StringsList& list) const
{
    auto obj = enumFieldDslObj();
    auto defaultValue = obj.defaultValue();

    if (defaultValue == 0) {
        return;
    }

    auto type = obj.type();
    if ((type == commsdsl::EnumField::Type::Uint64) ||
        (type == commsdsl::EnumField::Type::Uintvar)) {
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

void EnumField::checkValidRangesOpt(EnumField::StringsList& list) const
{
    static_cast<void>(list);
//    auto obj = enumFieldDslObj();
//    auto validRanges = obj.validRanges(); // copy
//    if (validRanges.empty()) {
//        return;
//    }

//    auto type = obj.type();
//    bool bigUnsigned =
//        (type == commsdsl::EnumField::Type::Uint64) ||
//        ((type != commsdsl::EnumField::Type::Uintvar) && (obj.maxLength() >= sizeof(std::int64_t)));

//    bool validCheckVersion =
//        generator().versionDependentCode() &&
//        obj.validCheckVersion();

//    if (!validCheckVersion) {
//        // unify
//        std::size_t idx = 0U;
//        while (idx < (validRanges.size() - 1U)) {
//            auto& thisRange = validRanges[idx];
//            auto& nextRange = validRanges[idx + 1];


//            auto needToMergeCheck =
//                [](auto min1, auto max1, auto min2, auto max2) -> bool
//                {
//                    assert(min1 <= min2);
//                    if (min2 <= (max1 + 1)) {
//                        assert(max1 <= max2);
//                        return true;
//                    }

//                    return false;
//                };

//            bool merge = false;
//            if (bigUnsigned) {
//                merge =
//                    needToMergeCheck(
//                        static_cast<std::uintmax_t>(thisRange.m_min),
//                        static_cast<std::uintmax_t>(thisRange.m_max),
//                        static_cast<std::uintmax_t>(nextRange.m_min),
//                        static_cast<std::uintmax_t>(nextRange.m_max));
//            }
//            else {
//                merge = needToMergeCheck(thisRange.m_min, thisRange.m_max, nextRange.m_min, nextRange.m_max);
//            }

//            if (!merge) {
//                ++idx;
//                continue;
//            }

//            auto needToUpdateCheck =
//                [](auto max1, auto max2) -> bool
//                {
//                    return max1 < max2;
//                };

//            bool update = false;
//            if (bigUnsigned) {
//                update =
//                    needToUpdateCheck(
//                        static_cast<std::uintmax_t>(thisRange.m_max),
//                        static_cast<std::uintmax_t>(nextRange.m_max));
//            }
//            else {
//                update = needToUpdateCheck(thisRange.m_max, nextRange.m_max);
//            }

//            if (update) {
//                thisRange.m_max = nextRange.m_max;
//            }

//            validRanges.erase(validRanges.begin() + idx + 1);
//        }
//    }

//    bool versionStorageRequired = false;
//    bool addedRangeOpt = false;
//    for (auto& r : validRanges) {
//        if (!generator().doesElementExist(r.m_sinceVersion, r.m_deprecatedSince, true)) {
//            continue;
//        }

//        if (validCheckVersion && (generator().isElementOptional(r.m_sinceVersion, r.m_deprecatedSince))) {
//            versionStorageRequired = true;
//            continue;
//        }

//        bool big = false;
//        std::string str = "comms::option::";
//        do {
//            if (!bigUnsigned) {
//                break;
//            }

//            bool minInRange =
//                static_cast<std::uintmax_t>(r.m_min) <= static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());

//            bool maxInRange =
//                static_cast<std::uintmax_t>(r.m_max) <= static_cast<std::uintmax_t>(std::numeric_limits<std::intmax_t>::max());
//            if (minInRange && maxInRange) {
//                break;
//            }

//            if (r.m_min == r.m_max) {
//                str += "ValidBigUnsignedNumValue<";
//                str += common::numToString(static_cast<std::uintmax_t>(r.m_min));
//                str += '>';
//            }
//            else {
//                str += "ValidBigUnsignedNumValueRange<";
//                str += common::numToString(static_cast<std::uintmax_t>(r.m_min));
//                str += ", ";
//                str += common::numToString(static_cast<std::uintmax_t>(r.m_max));
//                str += '>';
//            }

//            list.push_back(std::move(str));
//            big = true;
//            addedRangeOpt = true;
//        } while (false);

//        if (big) {
//            continue;
//        }

//        if (r.m_min == r.m_max) {
//            str += "ValidNumValue<";
//            str += common::numToString(r.m_min);
//            str += '>';
//        }
//        else {
//            str += "ValidNumValueRange<";
//            str += common::numToString(r.m_min);
//            str += ", ";
//            str += common::numToString(r.m_max);
//            str += '>';
//        }

//        list.push_back(std::move(str));
//        addedRangeOpt = true;
//    }

//    if (versionStorageRequired) {
//        list.push_back("comms::option::VersionStorage");

//        if (!addedRangeOpt) {
//            list.push_back("comms::option::InvalidByDefault");
//        }
//    }
}

} // namespace commsdsl2comms
