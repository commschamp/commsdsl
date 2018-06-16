#include "EnumField.h"

#include <type_traits>
#include <algorithm>

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

const std::size_t MaxRangesInOpts = 5U;

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

    common::StringsList valuesStrings;
    auto& values = obj.values();
    for (auto& v : obj.revValues()) {
        auto iter = values.find(v.second);
        if (iter == values.end()) {
            assert(!"Should not happen");
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

        valuesStrings.push_back(std::move(str));
    }
    return valuesStrings;
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

const std::string& EnumField::underlyingType() const
{
    auto obj = enumFieldDslObj();
    return IntField::convertType(obj.type());
}

bool EnumField::prepareImpl()
{
    auto obj = enumFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    auto& values = obj.values();
    m_validRanges.reserve(values.size());
    for (auto& v : values) {
        bool exists =
            generator().doesElementExist(
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
        generator().logger().error("Enum \"" + name() + "\" doesn't define any value");
        return false;
    }

    if (m_validRanges.size() <= 1U) {
        return true;
    }

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::EnumField::Type::Uint64) ||
        (type == commsdsl::EnumField::Type::Uintvar);


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

    // Sort by min/max value
    std::sort(
        m_validRanges.begin(), m_validRanges.end(),
        [bigUnsigned](auto& elem1, auto& elem2)
        {
            assert(elem1.m_deprecatedSince != 0U);
            assert(elem2.m_deprecatedSince != 0U);
            if (elem1.m_min != elem2.m_min) {
                if (bigUnsigned) {
                    return static_cast<std::uintmax_t>(elem1.m_min) < static_cast<std::uintmax_t>(elem2.m_min);
                }
                else {
                    return elem1.m_min < elem2.m_min;
                }
            }

            if (elem1.m_max != elem2.m_max) {
                if (bigUnsigned) {
                    return static_cast<std::uintmax_t>(elem1.m_max) < static_cast<std::uintmax_t>(elem2.m_max);
                }
                else {
                    return elem1.m_max < elem2.m_max;
                }
            }

            if (elem1.m_sinceVersion != elem2.m_sinceVersion) {
                return elem1.m_sinceVersion < elem2.m_sinceVersion;
            }

            assert(elem1.m_deprecatedSince != elem2.m_deprecatedSince);
            return elem1.m_deprecatedSince < elem2.m_deprecatedSince;
        });

    return true;
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

    if (MaxRangesInOpts < m_validRanges.size()) {
        common::mergeInclude("<iterator>", includes);
        common::mergeInclude("<algorithm>", includes);
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
        "/// @brief Values enumerator for @ref #^#NAME#$# field.\n"
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
    auto custom = getCustomValid();
    if (!custom.empty()) {
        return custom;
    }

    auto obj = enumFieldDslObj();

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    if (!validCheckVersion) {

        if (m_validRanges.size() <= MaxRangesInOpts) {
            return common::emptyString(); // Already in options
        }

        static const std::string Templ =
            "/// @brief Validity check function.\n"
            "bool valid() const\n"
            "{\n"
            "    if (!Base::valid()) {\n"
            "        return false;\n"
            "    }\n\n"
            "    static const typename Base::ValueType Values[] = {\n"
            "        #^#VALUES#$#\n"
            "    };\n\n"
            "    auto iter = std::find(std::begin(Values), std::end(Values), Base::value());\n"
            "    if ((iter == std::end(Values)) || (*iter != Base::value())) {\n"
            "        return false;\n"
            "    }\n\n"
            "    return true;\n"
            "}";

        common::StringsList valuesStrings;

        bool isVersion =
            obj.semanticType() == commsdsl::Field::SemanticType::MessageId;
        auto& revValues = obj.revValues();
        auto prevIter = revValues.end();
        for (auto iter = revValues.begin(); iter != revValues.end(); ++iter) {

            if ((prevIter != revValues.end()) && (prevIter->first == iter->first)) {
                continue;
            }

            std::string prefix;
            if (isVersion) {
                 prefix = generator().mainNamespace() + "::" + common::msgIdPrefixStr();
            }
            else {
                prefix = common::nameToClassCopy(name()) + "Val::";
            }

            valuesStrings.push_back(prefix + iter->second);
            prevIter = iter;
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("VALUES", common::listToString(valuesStrings, ",\n", common::emptyString())));
        return common::processTemplate(Templ, replacements);
    }

    assert(!"NYI");
    return common::emptyString();

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
    auto obj = enumFieldDslObj();

    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::EnumField::Type::Uint64) ||
        ((type != commsdsl::EnumField::Type::Uintvar) && (obj.maxLength() >= sizeof(std::int64_t)));

    bool validCheckVersion =
        generator().versionDependentCode() &&
        obj.validCheckVersion();

    auto addOptFunc =
        [&list, bigUnsigned](auto& r)
        {
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
                return;
            } while (false);

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
                    (elem.m_deprecatedSince == commsdsl::Protocol::notYetDeprecated());
            });

    if (uncondStartIter == m_validRanges.end()) {
        // No unconditionals found;
        list.push_back("comms::option::VersionStorage");
        list.push_back("comms::option::InvalidByDefault");
        return;
    }

    auto uncondEndIter =
        std::find_if(
            uncondStartIter + 1, m_validRanges.end(),
            [](auto& elem)
            {
                return
                    (elem.m_sinceVersion != 0U) ||
                    (elem.m_deprecatedSince != commsdsl::Protocol::notYetDeprecated());
            });

    auto uncondCount =
        static_cast<std::size_t>(std::distance(uncondStartIter, uncondEndIter));

    if (uncondCount != m_validRanges.size()) {
        list.push_back("comms::option::VersionStorage");
    }

    for (auto iter = uncondStartIter; iter != uncondEndIter; ++iter) {
        addOptFunc(*iter);
    }
}

} // namespace commsdsl2comms
