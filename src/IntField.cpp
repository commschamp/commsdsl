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
    "        #^#FIELD_TYPE#$#,\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::IntValue<\n"
    "            #^#PROT_NAMESPACE#$#::FieldBase<#^#FIELD_BASE_PARAMS#$#>,\n"
    "            #^#FIELD_TYPE#$#,\n"
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
    "        #^#FIELD_TYPE#$#,\n"
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

std::string IntField::getClassDefinitionImpl(const std::string& scope) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name())));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_BASE_PARAMS", getFieldBaseParams()));
    replacements.insert(std::make_pair("FIELD_TYPE", getFieldType()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getName()));

    static_cast<void>(scope);
    // TODO: more replacements

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

    if (!externalRef().empty()) {
        // TODO
        assert(!"NYI: add extra options");
    }

    options.push_back("typename TOpt::" + scope + common::nameToClassCopy(name()));
    checkLengthOpt(options);
    return common::listToString(options, ",\n", common::emptyString());
}

std::string IntField::getName() const
{
    return
        "/// @brief Name of the field.\n"
        "static const char* name()\n"
        "{\n"
        "    return \"" + getDisplayName() + "\"\n"
        "}\n";
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
        /* Int64 */ 5,
        /* Uint64 */ 5,
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

} // namespace commsdsl2comms
