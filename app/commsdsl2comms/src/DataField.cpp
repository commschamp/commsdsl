//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "DataField.h"

#include <type_traits>
#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string ClassTemplate(
    "#^#PREFIX_FIELD#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::ArrayList<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "        std::uint8_t#^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::ArrayList<\n"
    "            #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "            std::uint8_t#^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#CONSTRUCTOR#$#\n"
    "    #^#PUBLIC#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "#^#PROTECTED#$#\n"
    "#^#PRIVATE#$#\n"
    "};\n"
);

const std::string StructTemplate(
    "#^#PREFIX_FIELD#$#\n"
    "#^#PREFIX#$#"
    "struct #^#CLASS_NAME#$# : public\n"
    "    comms::field::ArrayList<\n"
    "        #^#PROT_NAMESPACE#$#::field::FieldBase<>,\n"
    "        std::uint8_t#^#COMMA#$#\n"
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
        hasNoValue("CONSTRUCTOR") &&
        hasNoValue("READ") &&
        hasNoValue("WRITE") &&
        hasNoValue("LENGTH") &&
        hasNoValue("VALID") &&
        hasNoValue("REFRESH") &&
        hasNoValue("PUBLIC") &&
        hasNoValue("PROTECTED") &&
        hasNoValue("PRIVATE");
}

} // namespace

bool DataField::prepareImpl()
{
    auto obj = dataFieldDslObj();
    if (!obj.hasLengthPrefixField()) {
        return true;
    }

    auto prefix = obj.lengthPrefixField();
    if (!prefix.externalRef().empty()) {
        return true;
    }

    m_prefix = create(generator(), prefix);
    if (!m_prefix->prepare(dslObj().sinceVersion())) {
        return false;
    }

    return true;
}

void DataField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/ArrayList.h",
        "<cstdint>"
    };

    common::mergeIncludes(List, includes);

    if (m_prefix) {
        m_prefix->updateIncludes(includes);
        return;
    }

    auto obj = dataFieldDslObj();
    if (obj.hasLengthPrefixField()) {
        auto prefix = obj.lengthPrefixField();
        assert(prefix.valid());
        auto prefixRef = prefix.externalRef();
        assert(!prefixRef.empty());
        common::mergeInclude(generator().headerfileForField(prefixRef, false), includes);
    }

    if (!obj.defaultValue().empty()) {
        common::mergeInclude("<iterator>", includes);
    }
}

std::size_t DataField::maxLengthImpl() const
{
    auto obj = dataFieldDslObj();
    if (obj.fixedLength() != 0U) {
        return Base::maxLengthImpl();
    }

    return std::numeric_limits<std::size_t>::max();
}

std::string DataField::getClassDefinitionImpl(
    const std::string& scope,
    const std::string& className) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(className)));
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getCustomRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getCustomRefresh()));
    replacements.insert(std::make_pair("CONSTRUCTOR", getConstructor(className)));
    replacements.insert(std::make_pair("PREFIX_FIELD", getPrefixField(scope)));
    replacements.insert(std::make_pair("PUBLIC", getExtraPublic()));
    replacements.insert(std::make_pair("PROTECTED", getFullProtected()));
    replacements.insert(std::make_pair("PRIVATE", getFullPrivate()));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements.insert(std::make_pair("COMMA", ","));
    }

    const std::string* templPtr = &ClassTemplate;
    if (shouldUseStruct(replacements)) {
        templPtr = &StructTemplate;
    }
    return common::processTemplate(*templPtr, replacements);
}

std::string DataField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    if (!m_prefix) {
        return common::emptyString();
    }

    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    auto fieldOptions = m_prefix->getDefaultOptions(memberScope);

    if (fieldOptions.empty()) {
        return common::emptyString();
    }

    const std::string Templ =
        "/// @brief Extra options for all the member fields of @ref #^#SCOPE#$##^#CLASS_NAME#$# string.\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#OPTIONS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("OPTIONS", std::move(fieldOptions)));
    return common::processTemplate(Templ, replacements);
}

std::string DataField::getCompareToValueImpl(
    const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    static_cast<void>(op);
    static_cast<void>(value);
    static_cast<void>(nameOverride);
    static_cast<void>(forcedVersionOptional);
    assert(!"Data field is not expected to be comparable");
    return common::emptyString();
}

std::string DataField::getCompareToFieldImpl(
    const std::string& op,
    const Field& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    static_cast<void>(op);
    static_cast<void>(field);
    static_cast<void>(nameOverride);
    static_cast<void>(forcedVersionOptional);
    assert(!"Data field is not expected to be comparable");
    return common::emptyString();
}

std::string DataField::getPrivateRefreshBodyImpl(const FieldsList& fields) const
{
    auto obj = dataFieldDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return common::emptyString();
    }

    auto iter =
        std::find_if(
            fields.begin(), fields.end(),
            [&detachedPrefixName](auto& f)
            {
                return f->name() == detachedPrefixName;
            });

    if (iter == fields.end()) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    bool lenVersionOptional = (*iter)->isVersionOptional();

    static const std::string Templ = 
        "auto expectedLength = static_cast<std::size_t>(field_#^#LEN_NAME#$#()#^#LEN_ACC#$#.value());\n"
        "auto realLength = field_#^#NAME#$#()#^#STR_ACC#$#.value().size();\n"
        "if (expectedLength != realLength) {\n"
        "    using LenValueType = typename std::decay<decltype(field_#^#LEN_NAME#$#()#^#LEN_ACC#$#.value())>::type;\n"
        "    field_#^#LEN_NAME#$#()#^#LEN_ACC#$#.value() = static_cast<LenValueType>(realLength);\n"
        "    return true;\n"
        "}\n\n"
        "return false;";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(name())));
    replacements.insert(std::make_pair("LEN_NAME", common::nameToAccessCopy(detachedPrefixName)));

    if (isVersionOptional()) {
        replacements.insert(std::make_pair("STR_ACC", ".field()"));
    }

    if (lenVersionOptional) {
        replacements.insert(std::make_pair("LEN_ACC", ".field()"));
    }
    
    return common::processTemplate(Templ, replacements);
}

bool DataField::hasCustomReadRefreshImpl() const
{
    return !dataFieldDslObj().detachedPrefixFieldName().empty();
}

std::string DataField::getReadPreparationImpl(const FieldsList& fields) const
{
    auto obj = dataFieldDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return common::emptyString();
    }

    bool versionOptional = isVersionOptional();

    auto iter =
        std::find_if(
            fields.begin(), fields.end(),
            [&detachedPrefixName](auto& f)
            {
                return f->name() == detachedPrefixName;
            });

    if (iter == fields.end()) {
        assert(!"Should not happen");
        return common::emptyString();
    }

    bool lenVersionOptional = (*iter)->isVersionOptional();

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NAME", common::nameToAccessCopy(name())));
    replacements.insert(std::make_pair("LEN_NAME", common::nameToAccessCopy(detachedPrefixName)));

    if ((!versionOptional) && (!lenVersionOptional)) {
        static const std::string Templ =
            "field_#^#NAME#$#().forceReadLength(\n"
            "    static_cast<std::size_t>(field_#^#LEN_NAME#$#().value()));\n";

        return common::processTemplate(Templ, replacements);
    }

    if ((versionOptional) && (!lenVersionOptional)) {
        static const std::string Templ =
            "if (field_#^#NAME#$#().doesExist()) {\n"
            "    field_#^#NAME#$#().field().forceReadLength(\n"
            "        static_cast<std::size_t>(field_#^#LEN_NAME#$#().value()));\n"
            "}\n";

        return common::processTemplate(Templ, replacements);
    }

    if ((!versionOptional) && (lenVersionOptional)) {
        static const std::string Templ =
            "if (field_#^#LEN_NAME#$#().doesExist()) {\n"
            "    field_#^#NAME#$#().forceReadLength(\n"
            "        static_cast<std::size_t>(field_#^#LEN_NAME#$#().field().value()));\n"
            "}\n";

        return common::processTemplate(Templ, replacements);
    }

    assert(versionOptional && lenVersionOptional);
    static const std::string Templ =
        "if (field_#^#NAME#$#().doesExist() && field_#^#LEN_NAME#$#().doesExist()) {\n"
        "    field_#^#NAME#$#().field().forceReadLength(\n"
        "        static_cast<std::size_t>(field_#^#LEN_NAME#$#().field().value()));\n"
        "}\n";

    return common::processTemplate(Templ, replacements);
}

bool DataField::isLimitedCustomizableImpl() const
{
    return true;
}

std::string DataField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);
    checkFixedLengthOpt(options);
    checkPrefixOpt(options);
    checkForcingOpt(options);

    return common::listToString(options, ",\n", common::emptyString());
}

std::string DataField::getConstructor(const std::string& className) const
{
    auto obj = dataFieldDslObj();
    auto& defaultValue = obj.defaultValue();
    if (defaultValue.empty()) {
        return common::emptyString();
    }

    static const std::string Templ =
        "/// @brief Default constructor\n"
        "#^#CLASS_NAME#$#()\n"
        "{\n"
        "    static const std::uint8_t Data[] = {\n"
        "        #^#BYTES#$#\n"
        "    };\n"
        "    Base::value().assign(std::begin(Data), std::end(Data));\n"
        "}\n";

    common::StringsList bytes;
    bytes.reserve(defaultValue.size());
    for (auto& b : defaultValue) {
        std::stringstream stream;
        stream << std::hex << "0x" << std::setfill('0') << std::setw(2) << static_cast<unsigned>(b);
        bytes.push_back(stream.str());
    }

    std::string bytesStr = common::listToString(bytes, ", ", common::emptyString());
    bytesStr = common::makeMultilineCopy(bytesStr);

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("BYTES", bytesStr));
    return common::processTemplate(Templ, replacements);
}

std::string DataField::getPrefixField(const std::string& scope) const
{
    if (!m_prefix) {
        return common::emptyString();
    }

    auto membersScope =
        scope + common::nameToClassCopy(name()) +
        common::membersSuffixStr() + "::";

    auto fieldDef = m_prefix->getClassDefinition(membersScope);
    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().mainNamespace() + "::" + common::defaultOptionsStr() + ">";
    }

    static const std::string Templ =
        "/// @brief Scope for all the member fields of @ref #^#CLASS_NAME#$# list.\n"
        "#^#EXTRA_PREFIX#$#\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#FIELD_DEF#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("FIELD_DEF", std::move(fieldDef)));
    return common::processTemplate(Templ, replacements);
}

void DataField::checkFixedLengthOpt(DataField::StringsList& list) const
{
    auto obj = dataFieldDslObj();
    auto fixedLen = obj.fixedLength();
    if (fixedLen == 0U) {
        return;
    }

    auto str =
        "comms::option::SequenceFixedSize<" +
        common::numToString(static_cast<std::uintmax_t>(fixedLen)) +
        ">";
    list.push_back(std::move(str));
}

void DataField::checkPrefixOpt(DataField::StringsList& list) const
{
    auto obj = dataFieldDslObj();
    if (!obj.hasLengthPrefixField()) {
        return;
    }

    std::string prefixName;
    if (m_prefix) {
        prefixName =
            "typename " +
            common::nameToClassCopy(name()) +
            common::membersSuffixStr();
        if (!externalRef().empty()) {
            prefixName += "<TOpt>";
        }

        prefixName += "::" + common::nameToClassCopy(m_prefix->name());
    }
    else {
        auto prefixField = obj.lengthPrefixField();
        assert(prefixField.valid());
        auto extRef = prefixField.externalRef();
        assert(!extRef.empty());
        prefixName = generator().scopeForField(extRef, true, true);
        prefixName += "<TOpt> ";
        auto* fieldPtr = generator().findField(extRef); // record usage
        assert(fieldPtr != nullptr);
        static_cast<void>(fieldPtr);
    }

    list.push_back("comms::option::SequenceSerLengthFieldPrefix<" + prefixName + '>');
}

void DataField::checkForcingOpt(StringsList& list) const
{
    auto obj = dataFieldDslObj();
    auto& detachedPrefixName = obj.detachedPrefixFieldName();
    if (detachedPrefixName.empty()) {
        return;
    }

    common::addToList("comms::option::SequenceLengthForcingEnabled", list);
}

} // namespace commsdsl2comms
