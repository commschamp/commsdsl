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

#include "CommsField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

bool hasOrigCode(commsdsl::parse::OverrideType value)
{
    return (value != commsdsl::parse::OverrideType_Replace);
}    

bool isOverrideCodeAllowed(commsdsl::parse::OverrideType value)
{
    return (value != commsdsl::parse::OverrideType_None);
}

bool isOverrideCodeRequired(commsdsl::parse::OverrideType value)
{
    return 
        (value == commsdsl::parse::OverrideType_Replace) || 
        (value == commsdsl::parse::OverrideType_Extend);
}

} // namespace 
    

CommsField::CommsField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

CommsField::~CommsField() = default;

CommsField::CommsFieldsList CommsField::commsTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields)
{
    CommsFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* commsField = 
            const_cast<CommsField*>(
                dynamic_cast<const CommsField*>(fPtr.get()));

        assert(commsField != nullptr);
        result.push_back(commsField);
    }

    return result;
}

bool CommsField::commsPrepare()
{
    auto& generator = m_field.generator();
    auto codePathPrefix = comms::inputCodePathFor(m_field, generator);

    auto& obj = m_field.dslObj();
    bool overrides = 
        commsPrepareOverrideInternal(obj.readOverride(), codePathPrefix, strings::readFileSuffixStr(), m_customRead, "read") &&
        commsPrepareOverrideInternal(obj.writeOverride(), codePathPrefix, strings::writeFileSuffixStr(), m_customWrite, "write") &&
        commsPrepareOverrideInternal(obj.refreshOverride(), codePathPrefix, strings::refreshFileSuffixStr(), m_customRefresh, "refresh") &&
        commsPrepareOverrideInternal(obj.lengthOverride(), codePathPrefix, strings::lengthFileSuffixStr(), m_customLength, "length") &&
        commsPrepareOverrideInternal(obj.validOverride(), codePathPrefix, strings::validFileSuffixStr(), m_customValid, "valid") &&
        commsPrepareOverrideInternal(obj.nameOverride(), codePathPrefix, strings::nameFileSuffixStr(), m_customName, "name");

    if (!overrides) {
        return false;
    }

    m_customExtend = util::readFileContents(codePathPrefix + strings::extendFileSuffixStr());
    return true;
}

bool CommsField::commsWrite() const
{
    auto* parent = m_field.getParent();
    if (parent == nullptr) {
        assert(false); // Should not happen
        return false;
    } 

    auto type = parent->elemType();
    if (type != commsdsl::gen::Elem::Type::Type_Namespace)
    {
        // Skip write for non-global fields,
        // The code generation will be driven by other means
        return true;
    }

    auto& dslObj = m_field.dslObj();
    if ((!dslObj.isForceGen()) && (!m_referenced)) {
        // Not referenced fields do not need to be written
        return true;
    }

    return 
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

CommsField::IncludesList CommsField::commsCommonIncludes() const
{
    return commsCommonIncludesImpl();
}

std::string CommsField::commsCommonCode() const
{
    auto base = commsCommonCodeBaseClassImpl();
    auto body = commsCommonCodeBodyImpl();
    std::string def;
    if ((!base.empty()) && (body.empty())) {
        static const std::string Templ = 
            "using #^#NAME#$#Common = #^#BASE#$#;";

        util::ReplacementMap repl = {
            {"NAME", comms::className(m_field.name())},
            {"BASE", std::move(base)},
        };

        def = util::processTemplate(Templ, repl);
    }
    else {
        static const std::string Templ = 
            "struct #^#NAME#$#Common#^#BASE#$#\n"
            "{\n"
            "    #^#BODY#$#\n"
            "};";

        util::ReplacementMap repl = {
            {"NAME", comms::className(m_field.name())},
            {"BASE", base.empty() ? strings::emptyString() : " : public " + base},
            {"BODY", std::move(body)}
        }; 
        def = util::processTemplate(Templ, repl);       
    }
    
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "/// @brief Common types and functions for\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "#^#DEF#$#\n"
        "#^#EXTRA#$#\n"
    ;

    auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"MEMBERS", commsCommonMembersCodeInternal()},
        {"SCOPE", comms::scopeFor(m_field, generator)},
        {"DEF", std::move(def)},
        {"EXTRA", commsCommonCodeExtraImpl()},
    };

    return util::processTemplate(Templ, repl);
}

bool CommsField::commsHasMembersCode() const
{
    return (!commsCommonMembersCodeImpl().empty()) || (!commsCommonMembersBaseClassImpl().empty());
}

bool CommsField::commsHasGeneratedReadCode() const
{
    return !commsDefReadFuncBodyImpl().empty();
}

bool CommsField::commsIsVersionDependent() const
{
    if (!m_field.generator().versionDependentCode()) {
        return false;
    }

    return (commsIsVersionOptional() || commsIsVersionDependentImpl());
}

std::size_t CommsField::commsMinLength() const
{
    if (commsIsVersionOptional()) {
        return 0U;
    }

    return commsMinLengthImpl();
}

std::size_t CommsField::commsMaxLength() const
{
    return commsMaxLengthImpl();
}

CommsField::IncludesList CommsField::commsDefIncludes() const
{
    auto& generator = m_field.generator();

    IncludesList list = {
        "comms/options.h",
        comms::relHeaderPathForField(strings::fieldBaseClassStr(), generator),
    };

    if (comms::isGlobalField(m_field)) {
        auto relHeader = comms::relCommonHeaderPathFor(m_field, generator);
        list.push_back(relHeader);
        list.push_back(comms::relHeaderForOptions(strings::defaultOptionsClassStr(), generator));
    }

    if (commsIsVersionOptional()) {
        list.push_back("comms/field/Optional.h");
    }

    auto extraList = commsDefIncludesImpl();
    list.insert(list.end(), extraList.begin(), extraList.end());
    
    return list;
}

std::string CommsField::commsDefCode() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#FIELD#$#\n"
        "#^#OPTIONAL#$#\n"
        "#^#APPEND#$#\n"
    ;

    //auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"MEMBERS", commsDefMembersCodeInternal()},
        {"FIELD", commsFieldDefCodeInternal()},
        {"OPTIONAL", commsOptionalDefCodeInternal()},
        {"APPEND", util::readFileContents(comms::inputCodePathFor(m_field, m_field.generator()) + strings::appendFileSuffixStr())}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefBundledReadPrepareFuncBody(const CommsFieldsList& siblings) const
{
    return commsDefBundledReadPrepareFuncBodyImpl(siblings);
}

std::string CommsField::commsDefBundledRefreshFuncBody(const CommsFieldsList& siblings) const
{
    return commsDefBundledRefreshFuncBodyImpl(siblings);
}

std::string CommsField::commsCompareToValueCode(
    const std::string& op,
    const std::string& value,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    return commsCompareToValueCodeImpl(op, value, nameOverride, forcedVersionOptional);
}

std::string CommsField::commsCompareToFieldCode(
    const std::string& op,
    const CommsField& field,
    const std::string& nameOverride,
    bool forcedVersionOptional) const
{
    return commsCompareToFieldCodeImpl(op, field, nameOverride, forcedVersionOptional);
}

bool CommsField::commsIsVersionOptional() const
{
    return comms::isVersionOptionaField(m_field, m_field.generator());
}

std::string CommsField::commsDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsDefaultOptions,
            nullptr,
            false);
}

std::string CommsField::commsDataViewDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsDataViewDefaultOptions,
            &CommsField::commsExtraDataViewDefaultOptionsInternal,
            true);
}

std::string CommsField::commsBareMetalDefaultOptions() const
{
    return 
        commsCustomizationOptionsInternal(
            &CommsField::commsBareMetalDefaultOptions,
            &CommsField::commsExtraBareMetalDefaultOptionsInternal,
            true);
}

CommsField::IncludesList CommsField::commsCommonIncludesImpl() const
{
    return IncludesList();
}

std::string CommsField::commsCommonCodeBaseClassImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsCommonCodeBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsCommonCodeExtraImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsCommonMembersBaseClassImpl() const
{
    return strings::emptyString();
}   

std::string CommsField::commsCommonMembersCodeImpl() const
{
    return strings::emptyString();
}

CommsField::IncludesList CommsField::commsDefIncludesImpl() const
{
    return IncludesList();
}

std::string CommsField::commsDefMembersCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefDoxigenDetailsImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefExtraDoxigenImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefBaseClassImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefPublicCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefProtectedCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefPrivateCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefReadFuncBodyImpl() const
{
    return strings::emptyString();
}

CommsField::StringsList CommsField::commsDefReadMsvcSuppressWarningsImpl() const
{
    return StringsList();
}

std::string CommsField::commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const
{
    static_cast<void>(siblings);
    return strings::emptyString();
}

std::string CommsField::commsDefWriteFuncBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefRefreshFuncBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const
{
    static_cast<void>(siblings);
    return strings::emptyString();
}

std::string CommsField::commsDefLengthFuncBodyImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsDefValidFuncBodyImpl() const
{
    return strings::emptyString();
}

bool CommsField::commsIsLimitedCustomizableImpl() const
{
    return false;
}

bool CommsField::commsIsVersionDependentImpl() const
{
    return false;
}

bool CommsField::commsDefHasNameFuncImpl() const
{
    return true;
}

std::string CommsField::commsCompareToValueCodeImpl(const std::string& op, const std::string& value, const std::string& nameOverride, bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = comms::accessName(m_field.dslObj().name());
    }

    bool versionOptional = forcedVersionOptional || commsIsVersionOptional();
    if (!versionOptional) {
        return
            "field_" + usedName + "().value() " +
            op + ' ' + value;
    }

    return
        "field_" + usedName + "().doesExist() &&\n" +
        "(field_" + usedName + "().field().value() " +
        op + ' ' + value + ')';
}

std::string CommsField::commsCompareToFieldCodeImpl(const std::string& op, const CommsField& field, const std::string& nameOverride, bool forcedVersionOptional) const
{
    auto usedName = nameOverride;
    if (usedName.empty()) {
        usedName = comms::accessName(m_field.dslObj().name());
    }

    auto fieldName = comms::accessName(field.m_field.dslObj().name());
    bool thisOptional = forcedVersionOptional || commsIsVersionOptional();
    bool otherOptional = field.commsIsVersionOptional();

    std::string thisFieldValue = "field_" + usedName + "()";
    if (thisOptional) {
        thisFieldValue += ".field()";
    }
    thisFieldValue += ".value() ";

    std::string otherFieldValue = " field_" + fieldName + "()";
    if (otherOptional) {
        otherFieldValue += ".field()";
    }
    otherFieldValue += ".value()";

    auto compareExpr = thisFieldValue + op + otherFieldValue;

    if ((!thisOptional) && (!otherOptional)) {
        return compareExpr;
    }

    if ((!thisOptional) && (otherOptional)) {
        return "field_" + fieldName + "().doesExist() &&\n(" + compareExpr + ')';
    }

    if ((thisOptional) && (!otherOptional)) {
        return "field_" + usedName + "().doesExist() &&\n(" + compareExpr + ')';
    }

    return
        "field_" + usedName + "().doesExist() &&\n"
        "field_" + fieldName + "().doesExist() &&\n"
                               "(" + compareExpr + ')';
}

std::string CommsField::commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const
{
    static_cast<void>(fieldOptsFunc);
    return strings::emptyString();
}

CommsField::StringsList CommsField::commsExtraDataViewDefaultOptionsImpl() const
{
    return StringsList();
}

CommsField::StringsList CommsField::commsExtraBareMetalDefaultOptionsImpl() const
{
    return StringsList();
}

std::size_t CommsField::commsMinLengthImpl() const
{
    return m_field.dslObj().minLength();
}

std::size_t CommsField::commsMaxLengthImpl() const
{
    return m_field.dslObj().maxLength();
}

std::string CommsField::commsCommonNameFuncCode() const
{
    auto& generator = m_field.generator();
    auto customNamePath = comms::inputCodePathFor(m_field, generator) + strings::nameFileSuffixStr();

    auto customFunc = util::readFileContents(customNamePath);
    if (!customFunc.empty()) {
        return customFunc;
    }

    static const std::string Templ = 
        "/// @brief Name of the @ref #^#SCOPE#$# field.\n"
        "static const char* name()\n"
        "{\n"
        "    return \"#^#NAME#$#\";\n"
        "}\n";

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(m_field, generator)},
        {"NAME", util::displayName(m_field.dslObj().displayName(), m_field.dslObj().name())},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsFieldBaseParams(commsdsl::parse::Endian endian) const
{
    auto& generator = m_field.generator();
    auto schemaEndian = generator.schemaEndian();
    assert(endian < commsdsl::parse::Endian_NumOfValues);
    assert(schemaEndian < commsdsl::parse::Endian_NumOfValues);

    if ((schemaEndian == endian) ||
        (commsdsl::parse::Endian_NumOfValues <= endian)) {
        return strings::emptyString();
    }

    return comms::dslEndianToOpt(endian);
}

void CommsField::commsAddFieldDefOptions(commsdsl::gen::util::StringsList& opts) const
{
    if (comms::isGlobalField(m_field)) {
        opts.push_back("TExtraOpts...");
    }

    if (commsIsFieldCustomizable()) {
        opts.push_back("typename TOpt::" + comms::scopeFor(m_field, m_field.generator(), false, true));
    }

    if (m_forcedFailOnInvalid) {
        opts.push_back("comms::option::def::FailOnInvalid<comms::ErrorStatus::ProtocolError>");
    }
    else if (m_field.dslObj().isFailOnInvalid()) {
        util::addToStrList("comms::option::def::FailOnInvalid<>", opts);
    }

    if (!m_customRead.empty()) {
        util::addToStrList("comms::option::def::HasCustomRead", opts);
    }

    if (!m_customRefresh.empty()) {
        util::addToStrList("comms::option::def::HasCustomRefresh", opts);
    }

    if (!m_customWrite.empty()) {
        util::addToStrList("comms::option::def::HasCustomWrite", opts);
    }    

    if (m_forcedPseudo || m_field.dslObj().isPseudo()) {
        util::addToStrList("comms::option::def::EmptySerialization", opts);
    }
}

bool CommsField::commsIsFieldCustomizable() const
{
    auto& generator = static_cast<CommsGenerator&>(m_field.generator());
    auto level = generator.getCustomizationLevel();
    if (level == CommsGenerator::CustomizationLevel::Full) {
        return true;
    }

    if (m_field.dslObj().isCustomizable()) {
        return true;
    }

    if (level == CommsGenerator::CustomizationLevel::None) {
        return false;
    }

    return commsIsLimitedCustomizableImpl();
}

bool CommsField::commsIsExtended() const
{
    return !m_customExtend.empty();
}

bool CommsField::commsPrepareOverrideInternal(
    commsdsl::parse::OverrideType type, 
    std::string& codePathPrefix, 
    const std::string& suffix,
    std::string& customCode,
    const std::string& name)
{
    if (isOverrideCodeRequired(type) && (!comms::isGlobalField(m_field))) {
        m_field.generator().logger().error(
            "Overriding \"" + name + "\" operation is not supported for non global fields, detected on \"" +
            comms::scopeFor(m_field, m_field.generator()) + "\".");
        return false;
    }

    if (isOverrideCodeAllowed(type)) {
        customCode = util::readFileContents(codePathPrefix + suffix);
    }    

    if (customCode.empty() && isOverrideCodeRequired(type)) {
        m_field.generator().logger().error(
            "Overriding \"" + name + "\" operation is not provided in injected code for field \"" +
            m_field.dslObj().externalRef() + "\".");
        return false;
    }

    return true;
}

bool CommsField::commsWriteCommonInternal() const
{
    auto& generator = m_field.generator();
    auto filePath = commsdsl::gen::comms::commonHeaderPathFor(m_field, generator);

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }

    auto includes = commsCommonIncludes();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains common template parameters independent functionality of\n"
        "///    @ref #^#FIELD_SCOPE#$# field.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n"
        "#^#NS_END#$#"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"FIELD_SCOPE", comms::scopeFor(m_field, generator)},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
        {"DEF", commsCommonCode()},
    };
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

bool CommsField::commsWriteDefInternal() const
{
    auto& generator = m_field.generator();
    auto filePath = commsdsl::gen::comms::headerPathFor(m_field, generator);

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    auto includes = commsDefIncludes();
    comms::prepareIncludeStatement(includes);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#FIELD_NAME#$#\"</b> field.\n"
        "\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#EXTRA_INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n"
        "#^#NS_END#$#"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"FIELD_NAME", util::displayName(m_field.dslObj().displayName(), m_field.dslObj().name())},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"EXTRA_INCLUDES", util::readFileContents(comms::inputCodePathFor(m_field, generator) + strings::incFileSuffixStr())},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
        {"DEF", commsDefCode()},
    };
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

std::string CommsField::commsFieldDefCodeInternal() const
{
    static const std::string Templ = 
        "#^#BRIEF#$#\n"
        "#^#DETAILS#$#\n"
        "#^#EXTRA_DOC#$#\n"
        "#^#DEPRECATED#$#\n"
        "#^#PARAMS#$#\n"
        "class #^#NAME#$##^#SUFFIX#$##^#ORIG#$# : public\n"
        "    #^#BASE#$#\n"
        "{\n"
        "    using Base =\n"
        "        #^#BASE#$#;\n"
        "#^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n"
        "#^#EXTEND#$#\n"
    ;

    //auto& generator = m_field.generator();

    auto pub = commsDefPublicCodeInternal();
    auto prot = commsDefProtectedCodeInternal();
    auto priv = commsDefPrivateCodeInternal();

    auto* templ = &Templ;
    if (pub.empty() && prot.empty() && priv.empty() && m_customExtend.empty()) {
        static const std::string AliasTempl = 
            "#^#BRIEF#$#\n"
            "#^#DETAILS#$#\n"
            "#^#EXTRA_DOC#$#\n"
            "#^#DEPRECATED#$#\n"
            "#^#PARAMS#$#\n"
            "using #^#NAME#$##^#SUFFIX#$# =\n"
            "    #^#BASE#$#;\n";

        templ = &AliasTempl;
    }

    util::ReplacementMap repl = {
        {"BRIEF", commsFieldBriefInternal()},
        {"DETAILS", commsDocDetailsInternal()},
        {"EXTRA_DOC", commsExtraDocInternal()},
        {"DEPRECATED", commsDeprecatedDocInternal()},
        {"PARAMS", commsTemplateParamsInternal()},
        {"NAME", comms::className(m_field.name())},
        {"BASE", commsDefBaseClassImpl()},
        {"PUBLIC", std::move(pub)},
        {"PROTECTED", std::move(prot)},
        {"PRIVATE", std::move(priv)},
        {"EXTEND", m_customExtend},
    };

    if (commsIsVersionOptional()) {
        repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();
    }
    
    if (!m_customExtend.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    return util::processTemplate(*templ, repl);
}

std::string CommsField::commsOptionalDefCodeInternal() const
{
    if (!commsIsVersionOptional()) {
        return strings::emptyString();
    }

    static const std::string Templ =
        "/// @brief Definition of version dependent\n"
        "///     <b>#^#NAME#$#</b> field."
        "#^#PARAMS#$#\n"
        "struct #^#CLASS_NAME#$# : public\n"
        "    comms::field::Optional<\n"
        "        #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#,\n"
        "        comms::option::def::#^#DEFAULT_MODE_OPT#$#,\n"
        "        comms::option::def::#^#VERSIONS_OPT#$#\n"
        "    >\n"
        "{\n"
        "    /// @brief Name of the field.\n"
        "    static const char* name()\n"
        "    {\n"
        "        return #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#::name();\n"
        "    }\n"
        "};\n"; 


        auto& generator = m_field.generator();
        auto& dslObj = m_field.dslObj();
        bool fieldExists = 
            generator.doesElementExist(
                dslObj.sinceVersion(),
                dslObj.deprecatedSince(),
                dslObj.isDeprecatedRemoved());

        std::string defaultModeOpt = "ExistsByDefault";
        if (!fieldExists) {
            defaultModeOpt = "MissingByDefault";
        }

        std::string versionOpt = "ExistsSinceVersion<" + util::numToString(dslObj.sinceVersion()) + '>';
        if (dslObj.isDeprecatedRemoved()) {
            assert(dslObj.deprecatedSince() < commsdsl::parse::Protocol::notYetDeprecated());
            if (dslObj.sinceVersion() == 0U) {
                versionOpt = "ExistsUntilVersion<" + util::numToString(dslObj.deprecatedSince() - 1) + '>';
            }
            else {
                versionOpt =
                    "ExistsBetweenVersions<" +
                    util::numToString(dslObj.sinceVersion()) +
                    ", " +
                    util::numToString(dslObj.deprecatedSince() - 1) +
                    '>';
            }
        }

    util::ReplacementMap repl = {
        {"NAME", util::displayName(dslObj.displayName(), dslObj.name())},
        {"PARAMS", commsTemplateParamsInternal()},
        {"CLASS_NAME", comms::className(dslObj.name())},
        {"DEFAULT_MODE_OPT", std::move(defaultModeOpt)},
        {"VERSIONS_OPT", std::move(versionOpt)},
    };

    if (comms::isGlobalField(m_field)) {
        repl.insert({{"FIELD_PARAMS", "<TOpt, TExtraOpts...>"}});
    }

    return util::processTemplate(Templ, repl); 
}

std::string CommsField::commsFieldBriefInternal() const
{
    if (commsIsVersionOptional()) {
        return "/// @brief Inner field of @ref " + comms::className(m_field.name()) + " optional.";
    }

    return
        "/// @brief Definition of <b>\"" +
        util::displayName(m_field.dslObj().displayName(), m_field.dslObj().name()) +
        "\"</b> field.";
}

std::string CommsField::commsDocDetailsInternal() const
{
    std::string result;
    do {
        auto& desc = m_field.dslObj().description();       
        auto extraDetails = commsDefDoxigenDetailsImpl();
        if (desc.empty() && extraDetails.empty()) {
            break;
        }

        result += "/// @details\n";

        if (!desc.empty()) {
            auto multiDesc = util::strMakeMultiline(desc);
            multiDesc = util::strInsertIndent(multiDesc);
            result += strings::doxygenPrefixStr() + util::strReplace(multiDesc, "\n", "\n" + strings::doxygenPrefixStr());
        }

        if (extraDetails.empty()) {
            break;
        }   

        result += '\n';      

        if (!desc.empty()) {
            result += strings::doxygenPrefixStr();
            result += '\n';
        }      

        auto multiExtra = util::strMakeMultiline(extraDetails);
        multiExtra = util::strInsertIndent(multiExtra);
        result += strings::doxygenPrefixStr() + util::strReplace(multiExtra, "\n", "\n" + strings::doxygenPrefixStr());
    } while (false);
    return result;
}

std::string CommsField::commsExtraDocInternal() const
{
    std::string result;
    auto doc = commsDefExtraDoxigenImpl();
    if (!doc.empty()) {
        result += strings::doxygenPrefixStr() + util::strReplace(doc, "\n", "\n" + strings::doxygenPrefixStr());
    }
    return result;
}

std::string CommsField::commsDeprecatedDocInternal() const
{
    std::string result;
    auto deprecatedVersion = m_field.dslObj().deprecatedSince();
    auto& generator = m_field.generator();
    if (generator.isElementDeprecated(deprecatedVersion)) {
        result += "/// @deprecated Since version " + std::to_string(deprecatedVersion) + '\n';
    }

    return result;
}

std::string CommsField::commsTemplateParamsInternal() const
{
    std::string result;
    if (comms::isGlobalField(m_field)) {
        result += "/// @tparam TOpt Protocol options.\n";
        result += "/// @tparam TExtraOpts Extra options.\n";
        result += "template <typename TOpt = ";
        result += comms::scopeForOptions(strings::defaultOptionsStr(), m_field.generator());
        result += ", typename... TExtraOpts>";
    }

    return result;    
}

std::string CommsField::commsDefPublicCodeInternal() const
{
    static const std::string Templ = {
        "public:\n"
        "    #^#FIELD_DEF#$#\n"
        "    #^#NAME#$#\n"
        "    #^#READ#$#\n"
        "    #^#WRITE#$#\n"
        "    #^#REFRESH#$#\n"
        "    #^#LENGTH#$#\n"
        "    #^#VALID#$#\n"
        "    #^#EXTRA_PUBLIC#$#\n"
    };

    auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"FIELD_DEF", commsDefPublicCodeImpl()},
        {"NAME", commsDefNameFuncCodeInternal()},
        {"READ", commsDefReadFuncCodeInternal()},
        {"WRITE", commsDefWriteFuncCodeInternal()},
        {"REFRESH", commsDefRefreshFuncCodeInternal()},
        {"LENGTH", commsDefLengthFuncCodeInternal()},
        {"VALID", commsDefValidFuncCodeInternal()},
        {"EXTRA_PUBLIC", util::readFileContents(comms::inputCodePathFor(m_field, generator) + strings::publicFileSuffixStr())},
    };

    bool hasValue = 
        std::any_of(
            repl.begin(), repl.end(),
            [](auto& elem)
            {
                return !elem.second.empty();
            });

    if (!hasValue) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefProtectedCodeInternal() const
{
    static const std::string Templ = 
        "protected:\n"
        "    #^#FIELD#$#\n"
        "    #^#CUSTOM#$#\n";

    util::ReplacementMap repl;

    auto field = commsDefProtectedCodeImpl();
    auto custom = util::readFileContents(comms::inputCodePathFor(m_field, m_field.generator()) + strings::protectedFileSuffixStr());
    
    if (!field.empty()) {
        repl.insert({{"FIELD", std::move(field)}});
    }

    if (!custom.empty()) {
        repl.insert({{"CUSTOM", std::move(custom)}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefPrivateCodeInternal() const
{
    static const std::string Templ = 
        "private:\n"
        "    #^#FIELD#$#\n"
        "    #^#CUSTOM#$#\n";

    util::ReplacementMap repl;

    auto field = commsDefPrivateCodeImpl();
    auto custom = util::readFileContents(comms::inputCodePathFor(m_field, m_field.generator()) + strings::privateFileSuffixStr());
    
    if (!field.empty()) {
        repl.insert({{"FIELD", std::move(field)}});
    }

    if (!custom.empty()) {
        repl.insert({{"CUSTOM", std::move(custom)}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefNameFuncCodeInternal() const
{
    auto& generator = m_field.generator();

    std::string custom;
    auto overrideType = m_field.dslObj().nameOverride();
    if (m_customName.empty() && (!commsDefHasNameFuncImpl())) {
        return strings::emptyString();
    }

    if (!hasOrigCode(overrideType)) {
        return m_customName;
    }

    static const std::string Templ = 
        "/// @brief Name of the field.\n"
        "static const char* name#^#SUFFIX#$#()\n"
        "{\n"
        "    return #^#SCOPE#$#::name();\n"
        "}\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl = {
        {"SCOPE", comms::commonScopeFor(m_field, generator)},
        {"CUSTOM", m_customName},
    };

    if (!repl["CUSTOM"].empty()) {
        repl["SUFFIX"] = strings::origSuffixStr();
    }
    
    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefReadFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    std::string body;
    if (hasOrigCode(m_field.dslObj().readOverride())) {
        body = commsDefReadFuncBodyImpl();
    }

    if (!body.empty()) {
        static const std::string OrigTempl = 
            "#^#MSVC_PUSH#$#\n"
            "#^#MSVC_DISABLE#$#\n"
            "/// @brief Generated read functionality.\n"
            "template <typename TIter>\n"
            "comms::ErrorStatus read#^#SUFFIX#$#(TIter& iter, std::size_t len)\n"
            "{\n"
            "    #^#BODY#$#\n"
            "}\n"
            "#^#MSVC_POP#$#\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customRead.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        auto warnings = commsDefReadMsvcSuppressWarningsImpl();
        if (!warnings.empty()) {
            util::StringsList disableStrings;
            for (auto& w : warnings) {
                disableStrings.push_back("COMMS_MSVC_WARNING_DISABLE(" + w + ')');
            }
            
            origRepl.insert({
                {"MSVC_PUSH", "COMMS_MSVC_WARNING_PUSH"},
                {"MSVC_POP", "COMMS_MSVC_WARNING_POP"},
                {"MSVC_DISABLE", util::strListToString(disableStrings, "\n", "")},
            });
        }

        repl["ORIG"] = util::processTemplate(OrigTempl, origRepl);
    }

    if (!m_customRead.empty()) {
        repl.insert({{"CUSTOM", m_customRead}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefWriteFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    std::string body;

    if (hasOrigCode(m_field.dslObj().writeOverride())) {
        body = commsDefWriteFuncBodyImpl();
    }

    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated write functionality.\n"
            "template <typename TIter>\n"
            "comms::ErrorStatus write#^#SUFFIX#$#(TIter& iter, std::size_t len) const\n"
            "{\n"
            "    #^#BODY#$#\n"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customWrite.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!m_customWrite.empty()) {
        repl.insert({{"CUSTOM", m_customWrite}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefRefreshFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    std::string body;
    if (hasOrigCode(m_field.dslObj().refreshOverride())) {
        body = commsDefRefreshFuncBodyImpl();
    }

    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated refresh functionality.\n"
            "bool refresh#^#SUFFIX#$#()\n"
            "{\n"
            "    #^#BODY#$#\n"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customRefresh.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!m_customRefresh.empty()) {
        repl.insert({{"CUSTOM", m_customRefresh}});
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefLengthFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;

    std::string body;
    if (hasOrigCode(m_field.dslObj().lengthOverride())) {
        body = commsDefLengthFuncBodyImpl();
    }

    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated length functionality.\n"
            "std::size_t length#^#SUFFIX#$#() const\n"
            "{\n"
            "    #^#BODY#$#\n"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customLength.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl.insert({{"ORIG", util::processTemplate(OrigTempl, origRepl)}});
    }

    if (!m_customLength.empty()) {
        repl["CUSTOM"] = m_customLength;
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefValidFuncCodeInternal() const
{
    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl;
    std::string body;
    if (hasOrigCode(m_field.dslObj().validOverride())) {
        body = commsDefValidFuncBodyImpl();
    }

    if (!body.empty()) {
        static const std::string OrigTempl = 
            "/// @brief Generated validity check functionality.\n"
            "bool valid#^#SUFFIX#$#() const\n"
            "{\n"
            "    #^#BODY#$#\n"
            "}\n";

        util::ReplacementMap origRepl = {
            {"BODY", std::move(body)}
        };

        if (!m_customValid.empty()) {
            origRepl["SUFFIX"] = strings::origSuffixStr();
        }

        repl["ORIG"] = util::processTemplate(OrigTempl, origRepl);
    }

    if (!m_customValid.empty()) {
        repl["CUSTOM"] = m_customValid;
    }

    if (repl.empty()) {
        return strings::emptyString();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsDefMembersCodeInternal() const
{
    auto body = commsDefMembersCodeImpl();
    if (body.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Scope for all the member fields of\n"
        "///     @ref #^#CLASS_NAME#$# field.\n"
        "#^#EXTRA_PREFIX#$#\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n";    

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(m_field.name())},
        {"BODY", std::move(body)}
    };

    if (comms::isGlobalField(m_field)) {
        auto prefix = 
            "/// @tparam TOpt Protocol options.\n"
            "template <typename TOpt = " + comms::scopeForOptions(strings::defaultOptionsStr(), m_field.generator()) + ">";
        
        repl.insert({{"EXTRA_PREFIX", std::move(prefix)}});
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsCommonMembersCodeInternal() const
{
    auto base = commsCommonMembersBaseClassImpl();
    auto body = commsCommonMembersCodeImpl();
    if (base.empty() && body.empty()) {
        return strings::emptyString();
    }

    if (body.empty()) {
        assert(!base.empty());
        static const std::string Templ = 
            "/// @brief Common definitions of the member fields of\n"
            "///     @ref #^#SCOPE#$# field.\n"
            "using #^#CLASS_NAME#$#MembersCommon = #^#BASE#$#;\n";        

        util::ReplacementMap repl = {
            {"SCOPE", comms::scopeFor(m_field, m_field.generator())},
            {"CLASS_NAME", comms::className(m_field.name())},
            {"BASE", std::move(base)}
        };

        return util::processTemplate(Templ, repl);            
    }

    static const std::string Templ = 
        "/// @brief Scope for all the common definitions of the member fields of\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "struct #^#CLASS_NAME#$#MembersCommon#^#BASE#$#\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n";    

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(m_field, m_field.generator())},
        {"CLASS_NAME", comms::className(m_field.name())},
        {"BODY", std::move(body)},
        {"BASE", base.empty() ? strings::emptyString() : " : public " + base}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsField::commsCustomizationOptionsInternal(
    FieldOptsFunc fieldOptsFunc, 
    ExtraFieldOptsFunc extraFieldOptsFunc,
    bool hasBase) const
{
    if ((!m_referenced) && (comms::isGlobalField(m_field))) {
        return strings::emptyString();
    }
    
    util::StringsList elems;
    auto membersBody = commsMembersCustomizationOptionsBodyImpl(fieldOptsFunc);
    if (!membersBody.empty()) {
        static const std::string Templ = 
            "struct #^#NAME#$##^#SUFFIX#$##^#EXT#$#\n"
            "{\n"
            "    #^#BODY#$#\n"
            "}; // struct #^#NAME#$##^#SUFFIX#$#\n";

        util::ReplacementMap repl = {
            {"NAME", comms::className(m_field.dslObj().name())},
            {"SUFFIX", strings::membersSuffixStr()},
            {"BODY", std::move(membersBody)},
        };

        if (hasBase) {
            repl["EXT"] = " : public TBase::" + comms::scopeFor(m_field, m_field.generator(), false) + strings::membersSuffixStr();
        }

        elems.push_back(util::processTemplate(Templ, repl));
    }

    do {
        if (!commsIsFieldCustomizable()) {
            break;
        }

        util::StringsList extraOpts;
        if (extraFieldOptsFunc != nullptr) {
            extraOpts = (this->*extraFieldOptsFunc)();
        }

        if (extraOpts.empty() && hasBase) {
            break;
        }        

        if (extraOpts.empty() && (!hasBase)) {
            extraOpts.push_back("comms::option::EmptyOption");
        }

        if ((!extraOpts.empty()) && (hasBase)) {
            extraOpts.push_back("typename TBase::" + comms::scopeFor(m_field, m_field.generator(), false));
        }

        auto docStr = 
            "/// @brief Extra options for @ref " +
            comms::scopeFor(m_field, m_field.generator()) + " field.";
        docStr = util::strMakeMultiline(docStr, 40);
        docStr = util::strReplace(docStr, "\n", "\n" + strings::doxygenPrefixStr() + strings::indentStr()); 

        util::ReplacementMap repl = {
            {"DOC", std::move(docStr)},
            {"NAME", comms::className(m_field.dslObj().name())},
        };        

        assert(!extraOpts.empty());
        if (extraOpts.size() == 1U) {
            static const std::string Templ = 
                "#^#DOC#$#\n"
                "using #^#NAME#$# = #^#OPT#$#;\n";
        
            repl["OPT"] = extraOpts.front();
            elems.push_back(util::processTemplate(Templ, repl));
            break;
        }    

        static const std::string Templ = 
            "#^#DOC#$#\n"
            "using #^#NAME#$# =\n"
            "    std::tuple<\n"
            "        #^#OPTS#$#\n"
            "    >;\n";
    
        repl["OPTS"] = util::strListToString(extraOpts, ",\n", "");
        elems.push_back(util::processTemplate(Templ, repl));
    } while (false);
    return util::strListToString(elems, "\n", "");
}

CommsField::StringsList CommsField::commsExtraDataViewDefaultOptionsInternal() const
{
    return commsExtraDataViewDefaultOptionsImpl();
}

CommsField::StringsList CommsField::commsExtraBareMetalDefaultOptionsInternal() const
{
    return commsExtraBareMetalDefaultOptionsImpl();
}

} // namespace commsdsl2comms
