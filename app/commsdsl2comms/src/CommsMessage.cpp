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

#include "CommsMessage.h"

#include "CommsField.h"
#include "CommsGenerator.h"
#include "CommsOptionalField.h"
#include "CommsSchema.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <numeric>
#include <utility>

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

void readCustomCodeInternal(const std::string& codePath, std::string& code)
{
    if (!util::isFileReadable(codePath)) {
        return;
    }

    code = util::readFileContents(codePath);
}

std::pair<const CommsField*, std::string> findInterfaceFieldInternal(const CommsGenerator& generator, const std::string& refStr)
{
    auto& currSchema = CommsSchema::cast(generator.currentSchema());
    auto* field = currSchema.findValidInterfaceReferencedField(refStr);
    if (field == nullptr) {
        generator.logger().error("Failed to find interface field \"" + refStr + "\".");
        assert(false);
        return std::make_pair(nullptr, std::string());
    }

    auto dotPos = refStr.find(".");
    if (refStr.size() < dotPos) {
        return std::make_pair(field, std::string());
    }

    return std::make_pair(field, refStr.substr(dotPos + 1));
}

std::string interfaceFieldAccStrInternal(const CommsField& field)
{
    return strings::transportFieldAccessPrefixStr() + comms::accessName(field.field().dslObj().name()) + "()";
}

void updateConstructBoolInternal(const CommsGenerator& generator, const commsdsl::parse::OptCondExpr& cond, util::StringsList& code)
{
    assert(cond.op().empty() || (cond.op() == "!"));
    auto& right = cond.right();
    assert(!right.empty());
    assert(right[0] == strings::interfaceFieldRefPrefix());

    auto lastSepPos = right.find_last_of(".");
    assert(lastSepPos < right.size());
    if (right.size() <= lastSepPos) {
        return;
    }

    auto fieldInfo = findInterfaceFieldInternal(generator, right.substr(1, lastSepPos - 1U));
    if (fieldInfo.first == nullptr) {
        assert(false); // Should not happen
        return;
    }

    auto fieldAccess = fieldInfo.first->commsFieldAccessStr(fieldInfo.second, interfaceFieldAccStrInternal(*fieldInfo.first));

    static const std::string TrueStr("true");
    static const std::string FalseStr("false");

    auto* valStr = &TrueStr;
    if (!cond.op().empty()) {
        valStr = &FalseStr;
    }

    static const std::string Templ = 
        "Base::#^#ACC#$#.setBitValue_#^#NAME#$#(#^#VAL#$#);\n";

    util::ReplacementMap repl = {
        {"ACC", std::move(fieldAccess)},
        {"NAME", right.substr(lastSepPos + 1U)},
        {"VAL", *valStr},
    };

    code.push_back(util::processTemplate(Templ, repl));
}

void updateConstructExprInternal(const CommsGenerator& generator, const commsdsl::parse::OptCondExpr& cond, util::StringsList& code)
{
    auto& left = cond.left();
    if (left.empty()) {
        updateConstructBoolInternal(generator, cond, code); 
        return;
    }

    assert(left[0] == strings::interfaceFieldRefPrefix());
    auto leftInfo = findInterfaceFieldInternal(generator, left.substr(1));
    if (leftInfo.first == nullptr) {
        assert(false); // Should not happen
        return;
    }
    
    assert(cond.op() == "=");
    auto& right = cond.right();
    assert(!right.empty());

    auto leftFieldAccess = leftInfo.first->commsFieldAccessStr(leftInfo.second, interfaceFieldAccStrInternal(*leftInfo.first));

    auto castPrefix = strings::transportFieldTypeAccessPrefixStr() + comms::accessName(leftInfo.first->field().dslObj().name()) + "::";
    std::string castType = leftInfo.first->commsCompValueCastType(leftInfo.second, castPrefix);
    std::string valStr;
    if (right[0] == strings::interfaceFieldRefPrefix()) {
        auto rightInfo = findInterfaceFieldInternal(generator, right.substr(1));
        if (rightInfo.first == nullptr) {
            assert(false); // Should not happen
            return;
        }

        valStr = rightInfo.first->commsValueAccessStr(rightInfo.second, interfaceFieldAccStrInternal(*rightInfo.first));
    }
    else {
        valStr = leftInfo.first->commsCompPrepValueStr(leftInfo.second, right);
    }

    static const std::string Templ = 
        "Base::#^#ACC#$#.value() = static_cast<typename Base::#^#CAST#$#>(#^#VAL#$#);\n";

    util::ReplacementMap repl = {
        {"ACC", std::move(leftFieldAccess)},
        {"CAST", std::move(castType)},
        {"VAL", std::move(valStr)}
    };

    code.push_back(util::processTemplate(Templ, repl));
}

void updateConstructCodeInternal(const CommsGenerator& generator, const commsdsl::parse::OptCond& cond, util::StringsList& code)
{
    assert(cond.valid());
    if (cond.kind() == commsdsl::parse::OptCond::Kind::Expr) {
        commsdsl::parse::OptCondExpr exprCond(cond);
        updateConstructExprInternal(generator, exprCond, code);
        return;
    }

    assert(cond.kind() == commsdsl::parse::OptCond::Kind::List);
    commsdsl::parse::OptCondList listCond(cond);
    assert(listCond.type() == commsdsl::parse::OptCondList::Type::And);
    auto conditions = listCond.conditions();
    for (auto& c : conditions) {
        updateConstructCodeInternal(generator, c, code);
    }
}

} // namespace 
    

CommsMessage::CommsMessage(CommsGenerator& generator, commsdsl::parse::Message dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
}   

CommsMessage::~CommsMessage() = default;

std::string CommsMessage::commsDefaultOptions() const
{
    return commsCustomizationOptionsInternal(&CommsField::commsDefaultOptions, nullptr, false);
}

std::string CommsMessage::commsClientDefaultOptions() const
{
    return commsCustomizationOptionsInternal(nullptr, &CommsMessage::commsClientExtraCustomizationOptionsInternal, true);
}

std::string CommsMessage::commsServerDefaultOptions() const
{
    return commsCustomizationOptionsInternal(nullptr, &CommsMessage::commsServerExtraCustomizationOptionsInternal, true);
}

std::string CommsMessage::commsDataViewDefaultOptions() const
{
    return commsCustomizationOptionsInternal(&CommsField::commsDataViewDefaultOptions, nullptr, true);    
}

std::string CommsMessage::commsBareMetalDefaultOptions() const
{
    return commsCustomizationOptionsInternal(&CommsField::commsBareMetalDefaultOptions, nullptr, true);    
}

bool CommsMessage::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    if (!copyCodeFromInternal()) {
        return false;
    }

    auto codePathPrefix = comms::inputCodePathFor(*this, generator());
    auto obj = dslObj();
    bool overrides = 
        commsPrepareOverrideInternal(obj.readOverride(), codePathPrefix, strings::readFileSuffixStr(), m_customCode.m_read, "read", &CommsMessage::commsPrepareCustomReadFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.writeOverride(), codePathPrefix, strings::writeFileSuffixStr(), m_customCode.m_write, "write", &CommsMessage::commsPrepareCustomWriteFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.refreshOverride(), codePathPrefix, strings::refreshFileSuffixStr(), m_customCode.m_refresh, "refresh", &CommsMessage::commsPrepareCustomRefreshFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.lengthOverride(), codePathPrefix, strings::lengthFileSuffixStr(), m_customCode.m_length, "length", &CommsMessage::commsPrepareCustomLengthFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.validOverride(), codePathPrefix, strings::validFileSuffixStr(), m_customCode.m_valid, "valid", &CommsMessage::commsPrepareCustomValidFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.nameOverride(), codePathPrefix, strings::nameFileSuffixStr(), m_customCode.m_name, "name", &CommsMessage::commsPrepareCustomNameFromBodyInternal);

    if (!overrides) {
        return false;
    }

    readCustomCodeInternal(codePathPrefix + strings::constructFileSuffixStr(), m_customConstruct);
    readCustomCodeInternal(codePathPrefix + strings::incFileSuffixStr(), m_customCode.m_inc);
    readCustomCodeInternal(codePathPrefix + strings::publicFileSuffixStr(), m_customCode.m_public);
    readCustomCodeInternal(codePathPrefix + strings::protectedFileSuffixStr(), m_customCode.m_protected);
    readCustomCodeInternal(codePathPrefix + strings::privateFileSuffixStr(), m_customCode.m_private);
    readCustomCodeInternal(codePathPrefix + strings::extendFileSuffixStr(), m_customCode.m_extend);
    readCustomCodeInternal(codePathPrefix + strings::appendFileSuffixStr(), m_customCode.m_append);

    m_commsFields = CommsField::commsTransformFieldsList(fields());
    m_bundledReadPrepareCodes.reserve(m_commsFields.size());
    m_bundledRefreshCodes.reserve(m_commsFields.size());
    for (auto* m : m_commsFields) {
        m_bundledReadPrepareCodes.push_back(m->commsDefBundledReadPrepareFuncBody(m_commsFields));
        m_bundledRefreshCodes.push_back(m->commsDefBundledRefreshFuncBody(m_commsFields));
    }  

    commsPrepareConstructCodeInternal();
    return true;
}

bool CommsMessage::writeImpl() const
{
    return 
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

bool CommsMessage::copyCodeFromInternal()
{
    auto obj = dslObj();
    auto& copyFrom = obj.copyCodeFrom();
    if (copyFrom.empty()) {
        return true;
    }

    auto* origMsg = generator().findMessage(copyFrom);
    if (origMsg == nullptr) {
        generator().logger().error(
            "Failed to find referenced field \"" + copyFrom + "\" for copying overriding code.");
        assert(false); // Should not happen
        return false;
    }

    auto* commsMsg = dynamic_cast<CommsMessage*>(origMsg);
    assert(commsMsg != nullptr);
    m_customCode = commsMsg->m_customCode;
    return true;
}

bool CommsMessage::commsPrepareOverrideInternal(
    commsdsl::parse::OverrideType type, 
    std::string& codePathPrefix, 
    const std::string& suffix,
    std::string& customCode,
    const std::string& name,
    BodyCustomCodeFunc bodyFunc)
{
    do {
        if (!isOverrideCodeAllowed(type)) {
            customCode.clear();
            break;
        }

        auto contents = util::readFileContents(codePathPrefix + suffix);
        if (!contents.empty()) {
            customCode = std::move(contents);
            break;
        }

        if (bodyFunc == nullptr) {
            break;
        }

        auto bodyContents = bodyFunc(codePathPrefix);
        if (!bodyContents.empty()) {
            customCode = std::move(bodyContents);
            break;
        }       
    } while (false);
    
    if (customCode.empty() && isOverrideCodeRequired(type)) {
        generator().logger().error(
            "Overriding \"" + name + "\" operation is not provided in injected code for message \"" +
            dslObj().externalRef() + "\". Expected overriding file is \"" + codePathPrefix + suffix + ".");
        return false;
    }

    return true;
}


std::string CommsMessage::commsPrepareCustomReadFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::readFileContents(codePathPrefix + strings::readBodyFileSuffixStr());
    if (contents.empty()) {
        return std::string();
    }
    
    static const std::string Templ = 
        "/// @brief Custom read functionality\n"
        "template <typename TIter>\n"
        "comms::ErrorStatus doRead(TIter& iter, std::size_t len)\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"BODY", std::move(contents)},
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomWriteFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::readFileContents(codePathPrefix + strings::writeBodyFileSuffixStr());
    if (contents.empty()) {
        return std::string();
    }
    
    static const std::string Templ = 
        "/// @brief Custom write functionality\n"
        "template <typename TIter>\n"
        "comms::ErrorStatus doWrite(TIter& iter, std::size_t len) const\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"BODY", std::move(contents)},
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomRefreshFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::readFileContents(codePathPrefix + strings::refreshBodyFileSuffixStr());
    if (contents.empty()) {
        return std::string();
    }
    
    static const std::string Templ = 
        "/// @brief Custom refresh functionality\n"
        "bool doRefresh()\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"BODY", std::move(contents)},
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomLengthFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::readFileContents(codePathPrefix + strings::lengthBodyFileSuffixStr());
    if (contents.empty()) {
        return std::string();
    }
    
    static const std::string Templ = 
        "/// @brief Custom length calculation functionality\n"
        "std::size_t doLength() const\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"BODY", std::move(contents)},
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomValidFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::readFileContents(codePathPrefix + strings::validBodyFileSuffixStr());
    if (contents.empty()) {
        return std::string();
    }
    
    static const std::string Templ = 
        "/// @brief Custom validity check functionality\n"
        "bool doValid() const\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"BODY", std::move(contents)},
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomNameFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::readFileContents(codePathPrefix + strings::nameBodyFileSuffixStr());
    if (contents.empty()) {
        return std::string();
    }
    
    static const std::string Templ = 
        "/// @brief Name of the message.\n"
        "static const char* doName()\n"
        "{\n"
        "    #^#BODY#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"BODY", std::move(contents)},
    };
    
    return util::processTemplate(Templ, repl);
}

bool CommsMessage::commsWriteCommonInternal() const
{
    auto& gen = generator();
    auto filePath = comms::commonHeaderPathFor(*this, gen);

    auto& logger = gen.logger();
    logger.info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains common template parameters independent functionality of\n"
        "///    @ref #^#SCOPE#$# message and its fields.\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "#^#FIELDS_COMMON#$#\n"
        "/// @brief Common types and functions of \n"
        "///     @ref #^#SCOPE#$# message.\n"
        "struct #^#NAME#$#Common\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n\n"
        "#^#NS_END#$#\n\n";        
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"SCOPE", comms::scopeFor(*this, gen)},
        {"INCLUDES", commsCommonIncludesInternal()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"FIELDS_COMMON", commsCommonFieldsCodeInternal()},
        {"NAME", comms::className(dslObj().name())},
        {"BODY", commsCommonBodyInternal()},
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool CommsMessage::commsWriteDefInternal() const
{
    auto& gen = generator();
    auto writeFunc = 
        [&gen](const std::string& filePath, const std::string& content)
        {
            auto& logger = gen.logger();
            logger.info("Generating " + filePath);

            std::ofstream stream(filePath);
            if (!stream) {
                logger.error("Failed to open \"" + filePath + "\" for writing.");
                return false;
            }
            
            stream << content;
            stream.flush();
            return stream.good();
        };
    
    auto genFilePath = comms::headerPathFor(*this, gen);
    auto codePathPrefix = comms::inputCodePathFor(*this, gen);
    auto replaceContent = util::readFileContents(codePathPrefix + strings::replaceFileSuffixStr());
    if (!replaceContent.empty()) {
        return writeFunc(genFilePath, replaceContent);
    }

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#MESSAGE_NAME#$#\"</b> message and its fields.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#CUSTOM_INCLUDES#$#\n\n"
        "#^#NS_BEGIN#$#\n"
        "/// @brief Fields of @ref #^#CLASS_NAME#$#.\n"
        "/// @tparam TOpt Extra options\n"
        "/// @see @ref #^#CLASS_NAME#$#\n"
        "/// @headerfile #^#MESSAGE_HEADERFILE#$#\n"
        "template <typename TOpt = #^#OPTIONS#$#>\n"
        "struct #^#CLASS_NAME#$#Fields\n"
        "{\n"
        "    #^#FIELDS_DEF#$#\n"
        "    /// @brief All the fields bundled in std::tuple.\n"
        "    using All = std::tuple<\n"
        "        #^#FIELDS_LIST#$#\n"
        "    >;\n"
        "};\n"
        "\n"
        "/// @brief Definition of <b>\"#^#MESSAGE_NAME#$#\"</b> message class.\n"
        "/// @details\n"
        "#^#DOC_DETAILS#$#\n"
        "///     See @ref #^#CLASS_NAME#$#Fields for definition of the fields this message contains.\n"
        "/// @tparam TMsgBase Base (interface) class.\n"
        "/// @tparam TOpt Extra options\n"
        "/// @headerfile #^#MESSAGE_HEADERFILE#$#\n"
        "#^#DEPRECATED#$#\n"
        "template <typename TMsgBase, typename TOpt = #^#OPTIONS#$#>\n"
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public\n"
        "    #^#BASE#$#\n"
        "{\n"
        "    // Redefinition of the base class type\n"
        "    using Base =\n"
        "        #^#BASE#$#;\n"
        "\n"
        "#^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "#^#NS_END#$#\n";
    
    auto obj = dslObj();
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"MESSAGE_NAME", util::displayName(obj.displayName(), obj.name())},
        {"INCLUDES", commsDefIncludesInternal()},
        {"CUSTOM_INCLUDES", m_customCode.m_inc},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"CLASS_NAME", comms::className(obj.name())},
        {"MESSAGE_HEADERFILE", comms::relHeaderPathFor(*this, gen)},
        {"OPTIONS", comms::scopeForOptions(strings::defaultOptionsClassStr(), gen)},
        {"FIELDS_DEF", commsDefFieldsCodeInternal()},
        {"FIELDS_LIST", commsDefFieldClassNamesListInternal()},
        {"DOC_DETAILS", commsDefDocDetailsInternal()},
        {"DEPRECATED", commsDefDeprecatedDocInternal()},
        {"BASE", commsDefBaseClassInternal()},
        {"PUBLIC", commsDefPublicInternal()},
        {"PROTECTED", commsDefProtectedInternal()},
        {"PRIVATE", commsDefPrivateInternal()},
        {"EXTEND", m_customCode.m_extend},
        {"APPEND", m_customCode.m_append}
    };

    if (!m_customCode.m_extend.empty()) {
        repl["SUFFIX"] = strings::origSuffixStr();
    }

    return writeFunc(genFilePath, util::processTemplate(Templ, repl, true));
}

std::string CommsMessage::commsCommonIncludesInternal() const
{
    util::StringsList includes;
    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);

        auto fIncludes = commsField->commsCommonIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }
    comms::prepareIncludeStatement(includes);    
    return util::strListToString(includes, "\n", "\n");
}

std::string CommsMessage::commsCommonBodyInternal() const
{
    // Body contains only name for now
    return commsCommonNameFuncInternal();
}

std::string CommsMessage::commsCommonNameFuncInternal() const
{
    static const std::string Templ = 
        "/// @brief Name of the @ref #^#SCOPE#$# message.\n"
        "static const char* name()\n"
        "{\n"
        "    return \"#^#NAME#$#\";\n"
        "}\n";  

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, generator())},
        {"NAME", util::displayName(dslObj().displayName(), dslObj().name())}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsCommonFieldsCodeInternal() const
{
    static const std::string Templ =
        "/// @brief Common types and functions for fields of \n"
        "///     @ref #^#SCOPE#$# message.\n"
        "/// @see #^#SCOPE#$#Fields\n"
        "struct #^#NAME#$#FieldsCommon\n"
        "{\n"
        "    #^#FIELDS_BODY#$#\n"
        "};\n";  

    util::StringsList fields;
    for (auto* cField : m_commsFields) {
        assert(cField != nullptr);
        auto def = cField->commsCommonCode();
        if (def.empty()) {
            continue;
        }

        fields.push_back(std::move(def));
    }

    if (fields.empty()) {
        return strings::emptyString();
    }

    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, generator())},
        {"NAME", comms::className(dslObj().name())},
        {"FIELDS_BODY", util::strListToString(fields, "\n", strings::emptyString())}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefIncludesInternal() const
{
    auto& gen = generator();
    util::StringsList includes = {
        "<tuple>",
        "comms/MessageBase.h",
        comms::relHeaderForOptions(strings::defaultOptionsStr(), gen),
        comms::relCommonHeaderPathFor(*this, gen),
    };

    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);

        auto fIncludes = commsField->commsDefIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }
    comms::prepareIncludeStatement(includes);    
    return util::strListToString(includes, "\n", "\n");
}

std::string CommsMessage::commsDefConstructInternal() const
{
    if (!m_customConstruct.empty()) {
        return m_customConstruct;
    }    

    if (m_internalConstruct.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "#^#CLASS_NAME#$##^#SUFFIX#$#()\n"
        "{\n"
        "    #^#CODE#$#\n"    
        "}\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"CODE", m_internalConstruct}
    };

    if (!m_customCode.m_extend.empty()) {
        repl["SUFFIX"] = strings::origSuffixStr();
    }    

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefFieldsCodeInternal() const
{
    util::StringsList fields;
    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);
        fields.push_back(commsField->commsDefCode());
    }
    return util::strListToString(fields, "\n", "");
}

std::string CommsMessage::commsDefFieldClassNamesListInternal() const
{
    util::StringsList names;
    for (auto& fPtr : fields()) {
        assert(fPtr);
        names.push_back(comms::className(fPtr->dslObj().name()));
    }
    return util::strListToString(names, ",\n", "");
}

std::string CommsMessage::commsDefDocDetailsInternal() const
{
    auto desc = util::strMakeMultiline(dslObj().description());
    if (!desc.empty()) {
        static const std::string DocPrefix = strings::doxygenPrefixStr() + strings::incFileSuffixStr();
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + DocPrefix);
        desc = util::strReplace(desc, "\n", DocNewLineRepl);
        desc += " @n";
    }
    return desc;    
}

std::string CommsMessage::commsDefDeprecatedDocInternal() const
{
    auto deprecatedVer = dslObj().deprecatedSince();
    if (!generator().isElementDeprecated(deprecatedVer)) {
        return strings::emptyString();
    }

    return "/// @deprecated Since version " + std::to_string(deprecatedVer) + '.';
}

std::string CommsMessage::commsDefBaseClassInternal() const
{
    static const std::string Templ = 
        "comms::MessageBase<\n"
        "    TMsgBase,\n"
        "    #^#CUSTOMIZATION_OPT#$#\n"
        "    comms::option::def::StaticNumIdImpl<#^#MESSAGE_ID#$#>,\n"
        "    comms::option::def::FieldsImpl<typename #^#CLASS_NAME#$#Fields<TOpt>::All>,\n"
        "    comms::option::def::MsgType<#^#CLASS_NAME#$##^#ORIG#$#<TMsgBase, TOpt> >,\n"
        "    comms::option::def::HasName#^#COMMA#$#\n"
        "    #^#EXTRA_OPTIONS#$#\n"
        ">";    

    util::ReplacementMap repl = {
        {"CUSTOMIZATION_OPT", commsDefCustomizationOptInternal()},
        {"MESSAGE_ID", util::numToStringWithHexComment(dslObj().id())},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"EXTRA_OPTIONS", commsDefExtraOptionsInternal()},
    };

    if (!repl["EXTRA_OPTIONS"].empty()) {
        repl["COMMA"] = ",";
    }

    if (!m_customCode.m_extend.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefCustomizationOptInternal() const
{
    std::string result;
    if (commsIsCustomizableInternal()) {
        auto& gen = static_cast<const CommsGenerator&>(generator());
        result = "typename TOpt::" + comms::scopeFor(*this, generator(), gen.commsHasMainNamespaceInOptions(), true) + ",";
    }
    return result;
}

std::string CommsMessage::commsDefExtraOptionsInternal() const
{
    util::StringsList opts;

    // Messages don't need / support comms::option::def::HasCustomRead option

    bool hasGeneratedRefresh = 
        std::any_of(
            m_bundledRefreshCodes.begin(), m_bundledRefreshCodes.end(),
            [](const std::string& code)
            {
                return !code.empty();
            });

    if ((!m_customCode.m_refresh.empty()) || hasGeneratedRefresh) {
        util::addToStrList("comms::option::def::HasCustomRefresh", opts);
    }

    auto obj = dslObj();
    if (obj.isFailOnInvalid()) {
        util::addToStrList("comms::option::def::FailOnInvalid<>", opts);
    }

    return util::strListToString(opts, ",\n", "");    
}

std::string CommsMessage::commsDefPublicInternal() const
{
    static const std::string Templ =
        "public:\n"
        "    #^#CONSTRUCT#$#\n"
        "    #^#ACCESS#$#\n"
        "    #^#ALIASES#$#\n"
        "    #^#LENGTH_CHECK#$#\n"
        "    #^#EXTRA#$#\n"
        "    #^#NAME#$#\n"
        "    #^#READ#$#\n"
        "    #^#WRITE#$#\n"
        "    #^#LENGTH#$#\n"
        "    #^#VALID#$#\n"
        "    #^#REFRESH#$#\n"
    ;

    auto inputCodePrefix = comms::inputCodePathFor(*this, generator());
    util::ReplacementMap repl = {
        {"CONSTRUCT", commsDefConstructInternal()},
        {"ACCESS", commsDefFieldsAccessInternal()},
        {"ALIASES", commsDefFieldsAliasesInternal()},
        {"LENGTH_CHECK", commsDefLengthCheckInternal()},
        {"EXTRA", m_customCode.m_public},
        {"NAME", commsDefNameFuncInternal()},
        {"READ", commsDefReadFuncInternal()},
        {"WRITE", m_customCode.m_write},
        {"LENGTH", m_customCode.m_length},
        {"VALID", commsDefValidFuncInternal()},
        {"REFRESH", commsDefRefreshFuncInternal()},
    };

    return util::processTemplate(Templ, repl);
}        

std::string CommsMessage::commsDefProtectedInternal() const
{
    if (m_customCode.m_protected.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "protected:\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::ReplacementMap repl = {
        {"CUSTOM", m_customCode.m_protected}
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefPrivateInternal() const
{
    assert(m_bundledReadPrepareCodes.size() == m_commsFields.size());
    assert(m_bundledRefreshCodes.size() == m_commsFields.size());
    util::StringsList reads;
    util::StringsList refreshes;
    for (auto idx = 0U; idx < m_commsFields.size(); ++idx) {
        auto& readCode = m_bundledReadPrepareCodes[idx];
        auto& refreshCode = m_bundledRefreshCodes[idx];

        if (readCode.empty() && refreshCode.empty()) {
            continue;
        }

        auto accName = comms::accessName(m_commsFields[idx]->field().dslObj().name());

        if (!readCode.empty()) {
            static const std::string Templ = 
                "void readPrepare_#^#ACC_NAME#$#()\n"
                "{\n"
                "    #^#CODE#$#\n"
                "}\n";

            util::ReplacementMap repl = {
                {"ACC_NAME", accName},
                {"CODE", readCode}
            };

            reads.push_back(util::processTemplate(Templ, repl));
        }

        if (!refreshCode.empty()) {
            static const std::string Templ = 
                "bool refresh_#^#ACC_NAME#$#()\n"
                "{\n"
                "    #^#CODE#$#\n"
                "}\n";

            util::ReplacementMap repl = {
                {"ACC_NAME", accName},
                {"CODE", refreshCode}
            };

            refreshes.push_back(util::processTemplate(Templ, repl));
        }
    }

    bool hasPrivateConstruct = 
        (!m_internalConstruct.empty()) && (!m_customConstruct.empty());

    if (reads.empty() && refreshes.empty() && m_customCode.m_private.empty() && (!hasPrivateConstruct)) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "private:\n"
        "    #^#CONSTRUCT#$#\n"
        "    #^#READS#$#\n"
        "    #^#REFRESHES#$#\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::ReplacementMap repl = {
        {"CONSTRUCT", commsDefPrivateConstructInternal()},
        {"READS", util::strListToString(reads, "\n", "")},
        {"REFRESHES", util::strListToString(refreshes, "\n", "")},
        {"CUSTOM", m_customCode.m_private}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefFieldsAccessInternal() const
{
    if (m_commsFields.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Provide names and allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_MSG_FIELDS_NAMES macro\n"
        "///     related to @b comms::MessageBase class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated values, types and functions are:\n"
        "#^#DOC#$#\n"
        "COMMS_MSG_FIELDS_NAMES(\n"
        "    #^#NAMES#$#\n"
        ");\n"
        ;

    util::StringsList docs;
    util::StringsList names;

    auto msgClassName = comms::className(dslObj().name());
    for (auto& fPtr : fields()) {
        assert(fPtr);
        auto& name = fPtr->dslObj().name();
        auto accName = comms::accessName(name);
        auto className = comms::className(name);

        static const std::string DocPrefix = strings::doxygenPrefixStr() + strings::indentStr();
        auto doc = 
            DocPrefix + "@li @b FieldIdx_" + accName + " index, @b Field_" + accName +
            " type and @b field_" + accName + "() access fuction\n" +
            DocPrefix + strings::indentStr() + "for @ref " + 
            msgClassName + strings::fieldsSuffixStr() + "::" + className + " field.";

        names.push_back(accName);
        docs.push_back(std::move(doc));
    }
    
    util::ReplacementMap repl = {
        {"DOC", util::strListToString(docs, "\n", "")},
        {"NAMES", util::strListToString(names, ",\n", "")},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefFieldsAliasesInternal() const
{
    auto aliases = dslObj().aliases();
    if (aliases.empty()) {
        return strings::emptyString();    
    }

    util::StringsList result;
    for (auto& a : aliases) {
        static const std::string Templ =
            "/// @brief Alias to a member field.\n"
            "/// @details\n"
            "#^#ALIAS_DESC#$#\n"
            "///     Generates field access alias function(s):\n"
            "///     @b field_#^#ALIAS_NAME#$#() -> <b>field_#^#ALIASED_FIELD_DOC#$#</b>\n"
            "COMMS_MSG_FIELD_ALIAS(#^#ALIAS_NAME#$#, #^#ALIASED_FIELD#$#);\n";

        auto& fieldName = a.fieldName();
        auto fieldSubNames = util::strSplitByAnyChar(fieldName, ".");
        for (auto& n : fieldSubNames) {
            n = comms::accessName(n);
        }

        auto desc = util::strMakeMultiline(a.description());
        if (!desc.empty()) {
            desc = strings::doxygenPrefixStr() + strings::indentStr() + desc + " @n";
            desc = util::strReplace(desc, "\n", "\n" + strings::doxygenPrefixStr() + strings::indentStr());
        }        

        util::ReplacementMap repl = {
            {"ALIAS_DESC", std::move(desc)},
            {"ALIAS_NAME", comms::accessName(a.name())},
            {"ALIASED_FIELD_DOC", util::strListToString(fieldSubNames, "().field_", "()")},
            {"ALIASED_FIELD", util::strListToString(fieldSubNames, ", ", "")}
        };

        result.push_back(util::processTemplate(Templ, repl));
    }
    
    return util::strListToString(result, "\n", "");
}

std::string CommsMessage::commsDefLengthCheckInternal() const
{
    bool hasCustomLength = 
        std::any_of(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto* f)
            {
                return f->commsHasCustomLength();
            });

    if (hasCustomLength) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "// Compile time check for serialisation length.\n"
        "static const std::size_t MsgMinLen = Base::doMinLength();\n"
        "#^#MAX_LEN#$#\n"
        "static_assert(MsgMinLen == #^#MIN_LEN_VAL#$#, \"Unexpected min serialisation length\");\n"
        "#^#MAX_LEN_ASSERT#$#\n"
    ;

    auto minLength =
        std::accumulate(
            m_commsFields.begin(), m_commsFields.end(), std::size_t(0),
            [](std::size_t soFar, auto* f)
            {
                return comms::addLength(soFar, f->commsMinLength());
            });
    auto maxLength =
        std::accumulate(
            m_commsFields.begin(), m_commsFields.end(), std::size_t(0),
            [](std::size_t soFar, auto* f)
            {
                return comms::addLength(soFar, f->commsMaxLength());
            });    

    util::ReplacementMap repl = {
        {"MIN_LEN_VAL", util::numToString(minLength)},
    };

    if (maxLength != comms::maxPossibleLength()) {
        repl.insert({
            {"MAX_LEN", "static const std::size_t MsgMaxLen = Base::doMaxLength();"},
            {"MAX_LEN_ASSERT", "static_assert(MsgMaxLen == " + util::numToString(maxLength) + ", \"Unexpected max serialisation length\");"}
        });
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefNameFuncInternal() const
{
    std::string origCode;
    if (hasOrigCode(dslObj().nameOverride())) {
        static const std::string Templ = 
            "/// @brief Name of the message.\n"
            "static const char* doName#^#ORIG#$#()\n"
            "{\n"
            "    return #^#SCOPE#$#::name();\n"
            "}\n";

        util::ReplacementMap repl = {
            {"SCOPE", comms::commonScopeFor(*this, generator())},
        };

        if (!m_customCode.m_name.empty()) {
            repl["ORIG"] = strings::origSuffixStr();
        }

        origCode = util::processTemplate(Templ, repl);
    }

    if (m_customCode.m_name.empty()) {
        return origCode;
    }

    static const std::string Templ = 
       "#^#ORIG#$#\n"
       "#^#CUSTOM#$#\n"
    ;
    
    util::ReplacementMap repl = {
        {"ORIG", std::move(origCode)},
        {"CUSTOM", m_customCode.m_name},
    };    

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefReadFuncInternal() const
{
    std::string origCode;
    do {
        if (!hasOrigCode(dslObj().readOverride())) {
            break;
        }

        auto readCond = commsDefReadConditionsCodeInternal();

        util::StringsList reads;
        assert(m_bundledReadPrepareCodes.size() == m_commsFields.size());
        int prevIdx = -1;

        static const std::string EsCheckStr = 
            "if (es != comms::ErrorStatus::Success) {\n"
            "    break;\n"
            "}\n";

        for (auto idx = 0U; idx < m_commsFields.size(); ++idx) {
            if (m_bundledReadPrepareCodes[idx].empty()) {
                continue;
            }

            auto accName = comms::accessName(m_commsFields[idx]->field().dslObj().name());
            auto prepStr = "readPrepare_" + accName + "();\n";
            if (idx == 0U) {
                reads.push_back(std::move(prepStr));
                continue;
            }

            if (prevIdx < 0) {
                auto str = 
                    "es = Base::template doReadUntilAndUpdateLen<FieldIdx_" + accName + ">(iter, len);\n" + 
                    EsCheckStr + '\n' +
                    prepStr;
                reads.push_back(std::move(str));
                prevIdx = idx;
                continue;
            }

            auto prevAcc = comms::accessName(m_commsFields[prevIdx]->field().dslObj().name());
            auto str = 
                "es = Base::template doReadFromUntilAndUpdateLen<FieldIdx_" + prevAcc + ", FieldIdx_" + accName + ">(iter, len);\n" + 
                EsCheckStr + '\n' +
                prepStr;
            reads.push_back(std::move(str));
            prevIdx = idx;        
        }

        if (readCond.empty() && reads.empty()) {
            break;
        }

        std::string readsCode;
        if (!reads.empty()) {
            if (prevIdx < 0) {
                // Only the first element has readPrepare()
                reads.push_back("es = Base::doRead(iter, len);\n");
            }
            else {
                auto prevAcc = comms::accessName(m_commsFields[prevIdx]->field().dslObj().name());
                reads.push_back("es = Base::template doReadFrom<FieldIdx_" + prevAcc + ">(iter, len);\n");
            }
                        
            static const std::string ReadsTempl = 
                "#^#UPDATE_VERSION#$#\n"
                "auto es = comms::ErrorStatus::Success;\n"
                "do {\n"
                "    #^#READS#$#\n"
                "} while (false);\n\n"
                "#^#FAIL_ON_INVALID#$#\n"
                "return es;\n";

            util::ReplacementMap readsRepl = {
                {"READS", util::strListToString(reads, "\n", "")},
                {"UPDATE_VERSION", generator().schemaOf(*this).versionDependentCode() ? "Base::doFieldsVersionUpdate();" : strings::emptyString()},
            };                

            if (dslObj().isFailOnInvalid()) {
                static const std::string FailOnInvalidTempl = 
                    "if (!#^#VALID_PREFIX#$#doValid()) {\n"
                    "    es = comms::ErrorStatus::InvalidMsgData;\n"
                    "}\n";

                util::ReplacementMap failOnInvalidRepl;

                bool hasGeneratedValid = commsDefValidFuncInternal().empty();
                if (hasGeneratedValid) {
                    failOnInvalidRepl["VALID_PREFIX"] = "Base::";
                }

                readsRepl["FAIL_ON_INVALID"] = util::processTemplate(FailOnInvalidTempl, failOnInvalidRepl);
            }

            readsCode = util::processTemplate(ReadsTempl, readsRepl);
        }
        else {
            readsCode = "return Base::doRead(iter, len);\n";
        }

        if (readCond.empty() && readsCode.empty()) {
            break;
        }


        static const std::string Templ = 
            "/// @brief Generated read functionality.\n"
            "template <typename TIter>\n"
            "comms::ErrorStatus doRead#^#ORIG#$#(TIter& iter, std::size_t len)\n"
            "{\n"
            "    #^#READ_COND#$#\n"
            "    #^#READS#$#\n"
            "}\n"
            ;        

        util::ReplacementMap repl = {
            {"READ_COND", std::move(readCond)},
            {"READS", std::move(readsCode)},
        };

        if (!m_customCode.m_read.empty()) {
            repl["ORIG"] = strings::origSuffixStr();
        }        
        
        origCode = util::processTemplate(Templ, repl);            
    } while (false);

    if (m_customCode.m_read.empty()) {
        return origCode;
    }

    static const std::string Templ = 
       "#^#ORIG#$#\n"
       "#^#CUSTOM#$#\n"
    ;
    
    util::ReplacementMap repl = {
        {"ORIG", std::move(origCode)},
        {"CUSTOM", m_customCode.m_read},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefRefreshFuncInternal() const
{
    std::string origCode;
    do {
        if (!hasOrigCode(dslObj().refreshOverride())) {
            break;
        }

        assert(m_commsFields.size() == m_bundledRefreshCodes.size());
        util::StringsList fields;
        for (auto idx = 0U; idx < m_commsFields.size(); ++idx) {
            auto& code = m_bundledRefreshCodes[idx];
            if (code.empty()) {
                continue;
            }

            auto accName = comms::accessName(m_commsFields[idx]->field().dslObj().name());
            fields.push_back("updated = refresh_" + accName + "() || updated;");
        }

        if (fields.empty()) {
            break;
        }
        
        static const std::string Templ = 
            "/// @brief Generated refresh functionality.\n"
            "bool doRefresh#^#ORIG#$#()\n"
            "{\n"
            "   bool updated = Base::doRefresh();\n"
            "   #^#FIELDS#$#\n"
            "   return updated;\n"
            "}\n";

        util::ReplacementMap repl = {
            {"FIELDS", util::strListToString(fields, "\n", "")},
        };

        if (!m_customCode.m_refresh.empty()) {
            repl["ORIG"] = strings::origSuffixStr();
        }

        origCode = util::processTemplate(Templ, repl);
    } while (false);

    if (m_customCode.m_refresh.empty()) {
        return origCode;
    }    

    static const std::string Templ = 
       "#^#ORIG#$#\n"
       "#^#CUSTOM#$#\n"
    ;
    
    util::ReplacementMap repl = {
        {"ORIG", std::move(origCode)},
        {"CUSTOM", m_customCode.m_refresh},
    };

    return util::processTemplate(Templ, repl);    
}

std::string CommsMessage::commsDefPrivateConstructInternal() const
{
    if (m_internalConstruct.empty()) {
        return strings::emptyString();
    }

    if (m_customConstruct.empty()) {
        // The construct code is already in the constructor
        return strings::emptyString();
    }

    static const std::string Templ = 
        "void constructOrig()\n"
        "{\n"
        "    #^#CODE#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"CODE", m_internalConstruct}
    };

    return util::processTemplate(Templ, repl);
}

bool CommsMessage::commsIsCustomizableInternal() const
{
    auto& gen = static_cast<const CommsGenerator&>(generator());
    auto level = gen.commsGetCustomizationLevel();
    if (level == CommsGenerator::CustomizationLevel::Full) {
        return true;
    }

    if (dslObj().isCustomizable()) {
        return true;
    }

    if (level == CommsGenerator::CustomizationLevel::None) {
        return false;
    }

    return dslObj().sender() != commsdsl::parse::Message::Sender::Both;
}

std::string CommsMessage::commsCustomizationOptionsInternal(
    FieldOptsFunc fieldOptsFunc,
    ExtraMessageOptsFunc extraMessageOptsFunc,
    bool hasBase) const
{
    util::StringsList fieldOpts;
    if (fieldOptsFunc != nullptr) {
        for (auto* f : m_commsFields) {
            auto str = (f->*fieldOptsFunc)();
            if (!str.empty()) {
                fieldOpts.push_back(std::move(str));
            }
        }
    }

    util::StringsList elems;
    if (!fieldOpts.empty()) {
        static const std::string Templ = 
            "/// @brief Extra options for fields of\n"
            "///     @ref #^#SCOPE#$# message.\n"        
            "struct #^#NAME#$##^#SUFFIX#$##^#EXT#$#\n"
            "{\n"
            "    #^#BODY#$#\n"
            "};\n";

        util::ReplacementMap repl = {
            {"NAME", comms::className(dslObj().name())},
            {"SUFFIX", strings::fieldsSuffixStr()},
            {"SCOPE", comms::scopeFor(*this, generator())},
            {"BODY", util::strListToString(fieldOpts, "\n", "")}
        };

        if (hasBase) {
            auto& commsGen = static_cast<const CommsGenerator&>(generator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();                        
            repl["EXT"] = " : public TBase::" + comms::scopeFor(*this, generator(), hasMainNs) + strings::fieldsSuffixStr();
        }

        elems.push_back(util::processTemplate(Templ, repl));
    }

    do {
        if (!commsIsCustomizableInternal()) {
            break;
        }

        StringsList extraOpts;
        if (extraMessageOptsFunc != nullptr) {
            extraOpts = (this->*extraMessageOptsFunc)();
        }

        if (extraOpts.empty() && hasBase) {
            break;
        }        

        if (extraOpts.empty() && (!hasBase)) {
            extraOpts.push_back("comms::option::app::EmptyOption");
        }

        if ((!extraOpts.empty()) && (hasBase)) {
            auto& commsGen = static_cast<const CommsGenerator&>(generator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();             
            extraOpts.push_back("typename TBase::" + comms::scopeFor(*this, generator(), hasMainNs));
        }        

        auto docStr = 
            "/// @brief Extra options for @ref " +
            comms::scopeFor(*this, generator()) + " message.";
        docStr = util::strMakeMultiline(docStr, 40);
        docStr = util::strReplace(docStr, "\n", "\n" + strings::doxygenPrefixStr() + strings::indentStr());         

        util::ReplacementMap repl = {
            {"DOC", std::move(docStr)},
            {"NAME", comms::className(dslObj().name())},
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

std::string CommsMessage::commsDefReadConditionsCodeInternal() const
{
    auto readCond = dslObj().readCond();
    if (!readCond.valid()) {
        return strings::emptyString();
    }

    auto& gen = CommsGenerator::cast(generator());
    auto str = 
        CommsOptionalField::commsDslCondToString(gen, CommsFieldsList(), readCond, true);

    if (str.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "bool validRead =\n"
        "    #^#CODE#$#;\n\n"
        "if (!validRead) {\n"
        "    return comms::ErrorStatus::InvalidMsgData;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"CODE", std::move(str)},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefOrigValidCodeInternal() const
{
    auto obj = dslObj();

    if ((!m_customCode.m_valid.empty()) && (!hasOrigCode(obj.validOverride()))) {
        return strings::emptyString();
    }

    auto cond = obj.validCond();
    if (!cond.valid()) {
        return strings::emptyString();
    }

    auto& gen = CommsGenerator::cast(generator());
    auto str = 
        CommsOptionalField::commsDslCondToString(gen, m_commsFields, cond, true);

    if (str.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "// Generated validity check functionality\n"
        "bool doValid#^#SUFFIX#$#() const\n"
        "{\n"
        "    if (!Base::doValid()) {\n"
        "        return false;\n"
        "    }\n\n"
        "    return\n"
        "        #^#CODE#$#;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"CODE", std::move(str)},
    };

    if (!m_customCode.m_valid.empty()) {
        repl["SUFFIX"] = strings::origSuffixStr();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefValidFuncInternal() const
{
    auto orig = commsDefOrigValidCodeInternal();
    if (m_customCode.m_valid.empty()) {
        return orig;
    }

    if (orig.empty()) {
        return m_customCode.m_valid;
    }

    static const std::string Templ = 
        "#^#ORIG#$#\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl = {
        {"ORIG", std::move(orig)},
        {"CUSTOM", m_customCode.m_valid}
    };

    return util::processTemplate(Templ, repl);
}

CommsMessage::StringsList CommsMessage::commsClientExtraCustomizationOptionsInternal() const
{
    auto sender = dslObj().sender();
    if (sender == commsdsl::parse::Message::Sender::Both) {
        return StringsList();
    }

    if (sender == commsdsl::parse::Message::Sender::Client) {
        return StringsList{
            "comms::option::app::NoReadImpl",
            "comms::option::app::NoDispatchImpl"
        };
    }

    assert (sender == commsdsl::parse::Message::Sender::Server);
    return StringsList{
        "comms::option::app::NoWriteImpl",
        "comms::option::app::NoRefreshImpl"
    };
}

CommsMessage::StringsList CommsMessage::commsServerExtraCustomizationOptionsInternal() const
{
    auto sender = dslObj().sender();
    if (sender == commsdsl::parse::Message::Sender::Both) {
        return StringsList();
    }

    if (sender == commsdsl::parse::Message::Sender::Client) {
        return StringsList{
            "comms::option::app::NoWriteImpl",
            "comms::option::app::NoRefreshImpl"
        };
    }

    assert (sender == commsdsl::parse::Message::Sender::Server);
    return StringsList{
        "comms::option::app::NoReadImpl",
        "comms::option::app::NoDispatchImpl"
    };
}

void CommsMessage::commsPrepareConstructCodeInternal()
{
    auto cond = dslObj().construct();
    if (!cond.valid()) {
        return;
    }

    StringsList code;
    updateConstructCodeInternal(CommsGenerator::cast(generator()), cond, code);

    if (code.empty()) {
        return;
    }

    m_internalConstruct = util::strListToString(code, "", "");
}

} // namespace commsdsl2comms
