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

bool hasOrigCode(commsdsl::parse::ParseOverrideType value)
{
    return (value != commsdsl::parse::ParseOverrideType_Replace);
}    

bool isOverrideCodeAllowed(commsdsl::parse::ParseOverrideType value)
{
    return (value != commsdsl::parse::ParseOverrideType_None);
}

bool isOverrideCodeRequired(commsdsl::parse::ParseOverrideType value)
{
    return 
        (value == commsdsl::parse::ParseOverrideType_Replace) || 
        (value == commsdsl::parse::ParseOverrideType_Extend);
}

void readCustomCodeInternal(const std::string& codePath, std::string& code)
{
    if (!util::genIsFileReadable(codePath)) {
        return;
    }

    code = util::genReadFileContents(codePath);
}

std::pair<const CommsField*, std::string> genFindInterfaceFieldInternal(const CommsGenerator& generator, const std::string& refStr)
{
    auto& currSchema = CommsSchema::cast(generator.genCurrentSchema());
    auto* field = currSchema.findValidInterfaceReferencedField(refStr);
    if (field == nullptr) {
        generator.genLogger().genError("Failed to find interface field \"" + refStr + "\".");
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
    return strings::genTransportFieldAccessPrefixStr() + comms::genAccessName(field.field().genParseObj().parseName()) + "()";
}

void updateConstructBoolInternal(const CommsGenerator& generator, const commsdsl::parse::ParseOptCondExpr& cond, util::GenStringsList& code)
{
    assert(cond.parseOp().empty() || (cond.parseOp() == "!"));
    auto& right = cond.parseRight();
    assert(!right.empty());
    assert(right[0] == strings::genInterfaceFieldRefPrefix());

    auto lastSepPos = right.find_last_of(".");
    assert(lastSepPos < right.size());
    if (right.size() <= lastSepPos) {
        return;
    }

    auto fieldInfo = genFindInterfaceFieldInternal(generator, right.substr(1, lastSepPos - 1U));
    if (fieldInfo.first == nullptr) {
        assert(false); // Should not happen
        return;
    }

    auto fieldAccess = fieldInfo.first->commsFieldAccessStr(fieldInfo.second, interfaceFieldAccStrInternal(*fieldInfo.first));

    static const std::string TrueStr("true");
    static const std::string FalseStr("false");

    auto* valStr = &TrueStr;
    if (!cond.parseOp().empty()) {
        valStr = &FalseStr;
    }

    static const std::string Templ = 
        "Base::#^#ACC#$#.setBitValue_#^#NAME#$#(#^#VAL#$#);\n";

    util::ReplacementMap repl = {
        {"ACC", std::move(fieldAccess)},
        {"NAME", right.substr(lastSepPos + 1U)},
        {"VAL", *valStr},
    };

    code.push_back(util::genProcessTemplate(Templ, repl));
}

void updateConstructExprInternal(const CommsGenerator& generator, const commsdsl::parse::ParseOptCondExpr& cond, util::GenStringsList& code)
{
    auto& left = cond.parseLeft();
    if (left.empty()) {
        updateConstructBoolInternal(generator, cond, code); 
        return;
    }

    assert(left[0] == strings::genInterfaceFieldRefPrefix());
    auto leftInfo = genFindInterfaceFieldInternal(generator, left.substr(1));
    if (leftInfo.first == nullptr) {
        assert(false); // Should not happen
        return;
    }
    
    assert(cond.parseOp() == "=");
    auto& right = cond.parseRight();
    assert(!right.empty());

    auto leftFieldAccess = leftInfo.first->commsFieldAccessStr(leftInfo.second, interfaceFieldAccStrInternal(*leftInfo.first));

    auto castPrefix = strings::genTransportFieldTypeAccessPrefixStr() + comms::genAccessName(leftInfo.first->field().genParseObj().parseName()) + "::";
    std::string castType = leftInfo.first->commsCompValueCastType(leftInfo.second, castPrefix);
    std::string valStr;
    if (right[0] == strings::genInterfaceFieldRefPrefix()) {
        auto rightInfo = genFindInterfaceFieldInternal(generator, right.substr(1));
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

    code.push_back(util::genProcessTemplate(Templ, repl));
}

void updateConstructCodeInternal(const CommsGenerator& generator, const commsdsl::parse::ParseOptCond& cond, util::GenStringsList& code)
{
    assert(cond.parseValid());
    if (cond.parseKind() == commsdsl::parse::ParseOptCond::ParseKind::Expr) {
        commsdsl::parse::ParseOptCondExpr exprCond(cond);
        updateConstructExprInternal(generator, exprCond, code);
        return;
    }

    assert(cond.parseKind() == commsdsl::parse::ParseOptCond::ParseKind::List);
    commsdsl::parse::ParseOptCondList listCond(cond);
    assert(listCond.parseType() == commsdsl::parse::ParseOptCondList::ParseType::And);
    auto conditions = listCond.parseConditions();
    for (auto& c : conditions) {
        updateConstructCodeInternal(generator, c, code);
    }
}

} // namespace 
    

CommsMessage::CommsMessage(CommsGenerator& generator, commsdsl::parse::ParseMessage dslObj, commsdsl::gen::GenElem* parent) :
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

bool CommsMessage::genPrepareImpl()
{
    if (!Base::genPrepareImpl()) {
        return false;
    }

    if (!copyCodeFromInternal()) {
        return false;
    }

    auto codePathPrefix = comms::genInputCodePathFor(*this, genGenerator());
    auto obj = genParseObj();
    bool overrides = 
        commsPrepareOverrideInternal(obj.parseReadOverride(), codePathPrefix, strings::genReadFileSuffixStr(), m_customCode.m_read, "read", &CommsMessage::commsPrepareCustomReadFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.parseWriteOverride(), codePathPrefix, strings::genWriteFileSuffixStr(), m_customCode.m_write, "write", &CommsMessage::commsPrepareCustomWriteFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.parseRefreshOverride(), codePathPrefix, strings::genRefreshFileSuffixStr(), m_customCode.m_refresh, "refresh", &CommsMessage::commsPrepareCustomRefreshFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.parseLengthOverride(), codePathPrefix, strings::genLengthFileSuffixStr(), m_customCode.m_length, "length", &CommsMessage::commsPrepareCustomLengthFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.parseValidOverride(), codePathPrefix, strings::genValidFileSuffixStr(), m_customCode.m_valid, "valid", &CommsMessage::commsPrepareCustomValidFromBodyInternal) &&
        commsPrepareOverrideInternal(obj.parseNameOverride(), codePathPrefix, strings::genNameFileSuffixStr(), m_customCode.m_name, "name", &CommsMessage::commsPrepareCustomNameFromBodyInternal);

    if (!overrides) {
        return false;
    }

    readCustomCodeInternal(codePathPrefix + strings::genConstructFileSuffixStr(), m_customConstruct);
    readCustomCodeInternal(codePathPrefix + strings::genIncFileSuffixStr(), m_customCode.m_inc);
    readCustomCodeInternal(codePathPrefix + strings::genPublicFileSuffixStr(), m_customCode.m_public);
    readCustomCodeInternal(codePathPrefix + strings::genProtectedFileSuffixStr(), m_customCode.m_protected);
    readCustomCodeInternal(codePathPrefix + strings::genPrivateFileSuffixStr(), m_customCode.m_private);
    readCustomCodeInternal(codePathPrefix + strings::genExtendFileSuffixStr(), m_customCode.m_extend);
    readCustomCodeInternal(codePathPrefix + strings::genAppendFileSuffixStr(), m_customCode.m_append);

    m_commsFields = CommsField::commsTransformFieldsList(genFields());
    m_bundledReadPrepareCodes.reserve(m_commsFields.size());
    m_bundledRefreshCodes.reserve(m_commsFields.size());
    for (auto* m : m_commsFields) {
        m_bundledReadPrepareCodes.push_back(m->commsDefBundledReadPrepareFuncBody(m_commsFields));
        m_bundledRefreshCodes.push_back(m->commsDefBundledRefreshFuncBody(m_commsFields));
    }  

    commsPrepareConstructCodeInternal();
    return true;
}

bool CommsMessage::genWriteImpl() const
{
    return 
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

bool CommsMessage::copyCodeFromInternal()
{
    auto obj = genParseObj();
    auto& copyFrom = obj.parseCopyCodeFrom();
    if (copyFrom.empty()) {
        return true;
    }

    auto* origMsg = genGenerator().genGindMessage(copyFrom);
    if (origMsg == nullptr) {
        genGenerator().genLogger().genError(
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
    commsdsl::parse::ParseOverrideType type, 
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

        auto contents = util::genReadFileContents(codePathPrefix + suffix);
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
        genGenerator().genLogger().genError(
            "Overriding \"" + name + "\" operation is not provided in injected code for message \"" +
            genParseObj().parseExternalRef() + "\". Expected overriding file is \"" + codePathPrefix + suffix + ".");
        return false;
    }

    return true;
}


std::string CommsMessage::commsPrepareCustomReadFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::genReadFileContents(codePathPrefix + strings::genReadBodyFileSuffixStr());
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
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomWriteFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::genReadFileContents(codePathPrefix + strings::genWriteBodyFileSuffixStr());
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
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomRefreshFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::genReadFileContents(codePathPrefix + strings::genRefreshBodyFileSuffixStr());
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
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomLengthFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::genReadFileContents(codePathPrefix + strings::genLengthBodyFileSuffixStr());
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
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomValidFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::genReadFileContents(codePathPrefix + strings::genValidBodyFileSuffixStr());
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
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsPrepareCustomNameFromBodyInternal(const std::string& codePathPrefix)
{
    auto contents = util::genReadFileContents(codePathPrefix + strings::genNameBodyFileSuffixStr());
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
    
    return util::genProcessTemplate(Templ, repl);
}

bool CommsMessage::commsWriteCommonInternal() const
{
    auto& gen = genGenerator();
    auto filePath = comms::genCommonHeaderPathFor(*this, gen);

    auto& logger = gen.genLogger();
    logger.genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        logger.genError("Failed to open \"" + filePath + "\" for writing.");
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
        {"SCOPE", comms::genScopeFor(*this, gen)},
        {"INCLUDES", commsCommonIncludesInternal()},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"FIELDS_COMMON", commsCommonFieldsCodeInternal()},
        {"NAME", comms::genClassName(genParseObj().parseName())},
        {"BODY", commsCommonBodyInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool CommsMessage::commsWriteDefInternal() const
{
    auto& gen = genGenerator();
    auto writeFunc = 
        [&gen](const std::string& filePath, const std::string& content)
        {
            auto& logger = gen.genLogger();
            logger.genInfo("Generating " + filePath);

            std::ofstream stream(filePath);
            if (!stream) {
                logger.genError("Failed to open \"" + filePath + "\" for writing.");
                return false;
            }
            
            stream << content;
            stream.flush();
            return stream.good();
        };
    
    auto genFilePath = comms::genHeaderPathFor(*this, gen);
    auto codePathPrefix = comms::genInputCodePathFor(*this, gen);
    auto replaceContent = util::genReadFileContents(codePathPrefix + strings::genReplaceFileSuffixStr());
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
    
    auto obj = genParseObj();
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"MESSAGE_NAME", util::genDisplayName(obj.parseDisplayName(), obj.parseName())},
        {"INCLUDES", commsDefIncludesInternal()},
        {"CUSTOM_INCLUDES", m_customCode.m_inc},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"CLASS_NAME", comms::genClassName(obj.parseName())},
        {"MESSAGE_HEADERFILE", comms::genRelHeaderPathFor(*this, gen)},
        {"OPTIONS", comms::genScopeForOptions(strings::genDefaultOptionsClassStr(), gen)},
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
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return writeFunc(genFilePath, util::genProcessTemplate(Templ, repl, true));
}

std::string CommsMessage::commsCommonIncludesInternal() const
{
    util::GenStringsList includes;
    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);

        auto fIncludes = commsField->commsCommonIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }
    comms::genPrepareIncludeStatement(includes);    
    return util::genStrListToString(includes, "\n", "\n");
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
        {"SCOPE", comms::genScopeFor(*this, genGenerator())},
        {"NAME", util::genDisplayName(genParseObj().parseDisplayName(), genParseObj().parseName())}
    };

    return util::genProcessTemplate(Templ, repl);
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

    util::GenStringsList fields;
    for (auto* cField : m_commsFields) {
        assert(cField != nullptr);
        auto def = cField->commsCommonCode();
        if (def.empty()) {
            continue;
        }

        fields.push_back(std::move(def));
    }

    if (fields.empty()) {
        return strings::genEmptyString();
    }

    util::ReplacementMap repl = {
        {"SCOPE", comms::genScopeFor(*this, genGenerator())},
        {"NAME", comms::genClassName(genParseObj().parseName())},
        {"FIELDS_BODY", util::genStrListToString(fields, "\n", strings::genEmptyString())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefIncludesInternal() const
{
    auto& gen = genGenerator();
    util::GenStringsList includes = {
        "<tuple>",
        "comms/MessageBase.h",
        comms::genRelHeaderForOptions(strings::genDefaultOptionsStr(), gen),
        comms::genRelCommonHeaderPathFor(*this, gen),
    };

    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);

        auto fIncludes = commsField->commsDefIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }
    comms::genPrepareIncludeStatement(includes);    
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CommsMessage::commsDefConstructInternal() const
{
    if (!m_customConstruct.empty()) {
        return m_customConstruct;
    }    

    if (m_internalConstruct.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "#^#CLASS_NAME#$##^#SUFFIX#$#()\n"
        "{\n"
        "    #^#CODE#$#\n"    
        "}\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"CODE", m_internalConstruct}
    };

    if (!m_customCode.m_extend.empty()) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }    

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefFieldsCodeInternal() const
{
    util::GenStringsList fields;
    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);
        fields.push_back(commsField->commsDefCode());
    }
    return util::genStrListToString(fields, "\n", "");
}

std::string CommsMessage::commsDefFieldClassNamesListInternal() const
{
    util::GenStringsList names;
    for (auto& fPtr : genFields()) {
        assert(fPtr);
        names.push_back(comms::genClassName(fPtr->genParseObj().parseName()));
    }
    return util::genStrListToString(names, ",\n", "");
}

std::string CommsMessage::commsDefDocDetailsInternal() const
{
    auto desc = util::genStrMakeMultiline(genParseObj().parseDescription());
    if (!desc.empty()) {
        static const std::string DocPrefix = strings::genDoxygenPrefixStr() + strings::genIncFileSuffixStr();
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + DocPrefix);
        desc = util::genStrReplace(desc, "\n", DocNewLineRepl);
        desc += " @n";
    }
    return desc;    
}

std::string CommsMessage::commsDefDeprecatedDocInternal() const
{
    auto deprecatedVer = genParseObj().parseDeprecatedSince();
    if (!genGenerator().genIsElementDeprecated(deprecatedVer)) {
        return strings::genEmptyString();
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
        {"MESSAGE_ID", util::genNumToStringWithHexComment(genParseObj().parseId())},
        {"CLASS_NAME", comms::genClassName(genParseObj().parseName())},
        {"EXTRA_OPTIONS", commsDefExtraOptionsInternal()},
    };

    if (!repl["EXTRA_OPTIONS"].empty()) {
        repl["COMMA"] = ",";
    }

    if (!m_customCode.m_extend.empty()) {
        repl["ORIG"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefCustomizationOptInternal() const
{
    std::string result;
    if (commsIsCustomizableInternal()) {
        auto& gen = static_cast<const CommsGenerator&>(genGenerator());
        result = "typename TOpt::" + comms::genScopeFor(*this, genGenerator(), gen.commsHasMainNamespaceInOptions(), true) + ",";
    }
    return result;
}

std::string CommsMessage::commsDefExtraOptionsInternal() const
{
    util::GenStringsList opts;

    // Messages don't need / support comms::option::def::HasCustomRead option

    bool hasGeneratedRefresh = 
        std::any_of(
            m_bundledRefreshCodes.begin(), m_bundledRefreshCodes.end(),
            [](const std::string& code)
            {
                return !code.empty();
            });

    if ((!m_customCode.m_refresh.empty()) || hasGeneratedRefresh) {
        util::genAddToStrList("comms::option::def::HasCustomRefresh", opts);
    }

    auto obj = genParseObj();
    if (obj.parseIsFailOnInvalid()) {
        util::genAddToStrList("comms::option::def::FailOnInvalid<>", opts);
    }

    return util::genStrListToString(opts, ",\n", "");    
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

    auto inputCodePrefix = comms::genInputCodePathFor(*this, genGenerator());
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

    return util::genProcessTemplate(Templ, repl);
}        

std::string CommsMessage::commsDefProtectedInternal() const
{
    if (m_customCode.m_protected.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "protected:\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::ReplacementMap repl = {
        {"CUSTOM", m_customCode.m_protected}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefPrivateInternal() const
{
    assert(m_bundledReadPrepareCodes.size() == m_commsFields.size());
    assert(m_bundledRefreshCodes.size() == m_commsFields.size());
    util::GenStringsList reads;
    util::GenStringsList refreshes;
    for (auto idx = 0U; idx < m_commsFields.size(); ++idx) {
        auto& readCode = m_bundledReadPrepareCodes[idx];
        auto& refreshCode = m_bundledRefreshCodes[idx];

        if (readCode.empty() && refreshCode.empty()) {
            continue;
        }

        auto accName = comms::genAccessName(m_commsFields[idx]->field().genParseObj().parseName());

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

            reads.push_back(util::genProcessTemplate(Templ, repl));
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

            refreshes.push_back(util::genProcessTemplate(Templ, repl));
        }
    }

    bool hasPrivateConstruct = 
        (!m_internalConstruct.empty()) && (!m_customConstruct.empty());

    if (reads.empty() && refreshes.empty() && m_customCode.m_private.empty() && (!hasPrivateConstruct)) {
        return strings::genEmptyString();
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
        {"READS", util::genStrListToString(reads, "\n", "")},
        {"REFRESHES", util::genStrListToString(refreshes, "\n", "")},
        {"CUSTOM", m_customCode.m_private}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefFieldsAccessInternal() const
{
    if (m_commsFields.empty()) {
        return strings::genEmptyString();
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

    util::GenStringsList docs;
    util::GenStringsList names;

    auto msgClassName = comms::genClassName(genParseObj().parseName());
    for (auto& fPtr : genFields()) {
        assert(fPtr);
        auto& name = fPtr->genParseObj().parseName();
        auto accName = comms::genAccessName(name);
        auto className = comms::genClassName(name);

        static const std::string DocPrefix = strings::genDoxygenPrefixStr() + strings::genIndentStr();
        auto doc = 
            DocPrefix + "@li @b FieldIdx_" + accName + " index, @b Field_" + accName +
            " type and @b field_" + accName + "() access fuction\n" +
            DocPrefix + strings::genIndentStr() + "for @ref " + 
            msgClassName + strings::genFieldsSuffixStr() + "::" + className + " field.";

        names.push_back(accName);
        docs.push_back(std::move(doc));
    }
    
    util::ReplacementMap repl = {
        {"DOC", util::genStrListToString(docs, "\n", "")},
        {"NAMES", util::genStrListToString(names, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefFieldsAliasesInternal() const
{
    auto aliases = genParseObj().parseAliases();
    if (aliases.empty()) {
        return strings::genEmptyString();    
    }

    util::GenStringsList result;
    for (auto& a : aliases) {
        static const std::string Templ =
            "/// @brief Alias to a member field.\n"
            "/// @details\n"
            "#^#ALIAS_DESC#$#\n"
            "///     Generates field access alias function(s):\n"
            "///     @b field_#^#ALIAS_NAME#$#() -> <b>field_#^#ALIASED_FIELD_DOC#$#</b>\n"
            "COMMS_MSG_FIELD_ALIAS(#^#ALIAS_NAME#$#, #^#ALIASED_FIELD#$#);\n";

        auto& fieldName = a.parseFieldName();
        auto fieldSubNames = util::genStrSplitByAnyChar(fieldName, ".");
        for (auto& n : fieldSubNames) {
            n = comms::genAccessName(n);
        }

        auto desc = util::genStrMakeMultiline(a.parseDescription());
        if (!desc.empty()) {
            desc = strings::genDoxygenPrefixStr() + strings::genIndentStr() + desc + " @n";
            desc = util::genStrReplace(desc, "\n", "\n" + strings::genDoxygenPrefixStr() + strings::genIndentStr());
        }        

        util::ReplacementMap repl = {
            {"ALIAS_DESC", std::move(desc)},
            {"ALIAS_NAME", comms::genAccessName(a.parseName())},
            {"ALIASED_FIELD_DOC", util::genStrListToString(fieldSubNames, "().field_", "()")},
            {"ALIASED_FIELD", util::genStrListToString(fieldSubNames, ", ", "")}
        };

        result.push_back(util::genProcessTemplate(Templ, repl));
    }
    
    return util::genStrListToString(result, "\n", "");
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
        return strings::genEmptyString();
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
                return comms::genAddLength(soFar, f->commsMinLength());
            });
    auto maxLength =
        std::accumulate(
            m_commsFields.begin(), m_commsFields.end(), std::size_t(0),
            [](std::size_t soFar, auto* f)
            {
                return comms::genAddLength(soFar, f->commsMaxLength());
            });    

    util::ReplacementMap repl = {
        {"MIN_LEN_VAL", util::genNumToString(minLength)},
    };

    if (maxLength != comms::genMaxPossibleLength()) {
        repl.insert({
            {"MAX_LEN", "static const std::size_t MsgMaxLen = Base::doMaxLength();"},
            {"MAX_LEN_ASSERT", "static_assert(MsgMaxLen == " + util::genNumToString(maxLength) + ", \"Unexpected max serialisation length\");"}
        });
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefNameFuncInternal() const
{
    std::string origCode;
    if (hasOrigCode(genParseObj().parseNameOverride())) {
        static const std::string Templ = 
            "/// @brief Name of the message.\n"
            "static const char* doName#^#ORIG#$#()\n"
            "{\n"
            "    return #^#SCOPE#$#::name();\n"
            "}\n";

        util::ReplacementMap repl = {
            {"SCOPE", comms::genCommonScopeFor(*this, genGenerator())},
        };

        if (!m_customCode.m_name.empty()) {
            repl["ORIG"] = strings::genOrigSuffixStr();
        }

        origCode = util::genProcessTemplate(Templ, repl);
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

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefReadFuncInternal() const
{
    std::string origCode;
    do {
        if (!hasOrigCode(genParseObj().parseReadOverride())) {
            break;
        }

        auto readCond = commsDefReadConditionsCodeInternal();

        util::GenStringsList reads;
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

            auto accName = comms::genAccessName(m_commsFields[idx]->field().genParseObj().parseName());
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

            auto prevAcc = comms::genAccessName(m_commsFields[prevIdx]->field().genParseObj().parseName());
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
                auto prevAcc = comms::genAccessName(m_commsFields[prevIdx]->field().genParseObj().parseName());
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
                {"READS", util::genStrListToString(reads, "\n", "")},
                {"UPDATE_VERSION", genGenerator().genSchemaOf(*this).genVersionDependentCode() ? "Base::doFieldsVersionUpdate();" : strings::genEmptyString()},
            };                

            if (genParseObj().parseIsFailOnInvalid()) {
                static const std::string FailOnInvalidTempl = 
                    "if (!#^#VALID_PREFIX#$#doValid()) {\n"
                    "    es = comms::ErrorStatus::InvalidMsgData;\n"
                    "}\n";

                util::ReplacementMap failOnInvalidRepl;

                bool hasGeneratedValid = commsDefValidFuncInternal().empty();
                if (hasGeneratedValid) {
                    failOnInvalidRepl["VALID_PREFIX"] = "Base::";
                }

                readsRepl["FAIL_ON_INVALID"] = util::genProcessTemplate(FailOnInvalidTempl, failOnInvalidRepl);
            }

            readsCode = util::genProcessTemplate(ReadsTempl, readsRepl);
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
            repl["ORIG"] = strings::genOrigSuffixStr();
        }        
        
        origCode = util::genProcessTemplate(Templ, repl);            
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

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefRefreshFuncInternal() const
{
    std::string origCode;
    do {
        if (!hasOrigCode(genParseObj().parseRefreshOverride())) {
            break;
        }

        assert(m_commsFields.size() == m_bundledRefreshCodes.size());
        util::GenStringsList fields;
        for (auto idx = 0U; idx < m_commsFields.size(); ++idx) {
            auto& code = m_bundledRefreshCodes[idx];
            if (code.empty()) {
                continue;
            }

            auto accName = comms::genAccessName(m_commsFields[idx]->field().genParseObj().parseName());
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
            {"FIELDS", util::genStrListToString(fields, "\n", "")},
        };

        if (!m_customCode.m_refresh.empty()) {
            repl["ORIG"] = strings::genOrigSuffixStr();
        }

        origCode = util::genProcessTemplate(Templ, repl);
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

    return util::genProcessTemplate(Templ, repl);    
}

std::string CommsMessage::commsDefPrivateConstructInternal() const
{
    if (m_internalConstruct.empty()) {
        return strings::genEmptyString();
    }

    if (m_customConstruct.empty()) {
        // The construct code is already in the constructor
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "void constructOrig()\n"
        "{\n"
        "    #^#CODE#$#\n"
        "}\n";

    util::ReplacementMap repl = {
        {"CODE", m_internalConstruct}
    };

    return util::genProcessTemplate(Templ, repl);
}

bool CommsMessage::commsIsCustomizableInternal() const
{
    auto& gen = static_cast<const CommsGenerator&>(genGenerator());
    auto level = gen.commsGetCustomizationLevel();
    if (level == CommsGenerator::CustomizationLevel::Full) {
        return true;
    }

    if (genParseObj().parseIsCustomizable()) {
        return true;
    }

    if (level == CommsGenerator::CustomizationLevel::None) {
        return false;
    }

    return genParseObj().parseSender() != commsdsl::parse::ParseMessage::ParseSender::Both;
}

std::string CommsMessage::commsCustomizationOptionsInternal(
    FieldOptsFunc fieldOptsFunc,
    ExtraMessageOptsFunc extraMessageOptsFunc,
    bool hasBase) const
{
    util::GenStringsList fieldOpts;
    if (fieldOptsFunc != nullptr) {
        for (auto* f : m_commsFields) {
            auto str = (f->*fieldOptsFunc)();
            if (!str.empty()) {
                fieldOpts.push_back(std::move(str));
            }
        }
    }

    util::GenStringsList elems;
    if (!fieldOpts.empty()) {
        static const std::string Templ = 
            "/// @brief Extra options for fields of\n"
            "///     @ref #^#SCOPE#$# message.\n"        
            "struct #^#NAME#$##^#SUFFIX#$##^#EXT#$#\n"
            "{\n"
            "    #^#BODY#$#\n"
            "};\n";

        util::ReplacementMap repl = {
            {"NAME", comms::genClassName(genParseObj().parseName())},
            {"SUFFIX", strings::genFieldsSuffixStr()},
            {"SCOPE", comms::genScopeFor(*this, genGenerator())},
            {"BODY", util::genStrListToString(fieldOpts, "\n", "")}
        };

        if (hasBase) {
            auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();                        
            repl["EXT"] = " : public TBase::" + comms::genScopeFor(*this, genGenerator(), hasMainNs) + strings::genFieldsSuffixStr();
        }

        elems.push_back(util::genProcessTemplate(Templ, repl));
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
            auto& commsGen = static_cast<const CommsGenerator&>(genGenerator());
            bool hasMainNs = commsGen.commsHasMainNamespaceInOptions();             
            extraOpts.push_back("typename TBase::" + comms::genScopeFor(*this, genGenerator(), hasMainNs));
        }        

        auto docStr = 
            "/// @brief Extra options for @ref " +
            comms::genScopeFor(*this, genGenerator()) + " message.";
        docStr = util::genStrMakeMultiline(docStr, 40);
        docStr = util::genStrReplace(docStr, "\n", "\n" + strings::genDoxygenPrefixStr() + strings::genIndentStr());         

        util::ReplacementMap repl = {
            {"DOC", std::move(docStr)},
            {"NAME", comms::genClassName(genParseObj().parseName())},
        };        

        assert(!extraOpts.empty());
        if (extraOpts.size() == 1U) {
            static const std::string Templ = 
                "#^#DOC#$#\n"
                "using #^#NAME#$# = #^#OPT#$#;\n";
        
            repl["OPT"] = extraOpts.front();
            elems.push_back(util::genProcessTemplate(Templ, repl));
            break;
        }    

        static const std::string Templ = 
            "#^#DOC#$#\n"
            "using #^#NAME#$# =\n"
            "    std::tuple<\n"
            "        #^#OPTS#$#\n"
            "    >;\n";
    
        repl["OPTS"] = util::genStrListToString(extraOpts, ",\n", "");
        elems.push_back(util::genProcessTemplate(Templ, repl));

    } while (false);
    return util::genStrListToString(elems, "\n", "");
}

std::string CommsMessage::commsDefReadConditionsCodeInternal() const
{
    auto readCond = genParseObj().parseReadCond();
    if (!readCond.parseValid()) {
        return strings::genEmptyString();
    }

    auto& gen = CommsGenerator::cast(genGenerator());
    auto str = 
        CommsOptionalField::commsDslCondToString(gen, CommsFieldsList(), readCond, true);

    if (str.empty()) {
        return strings::genEmptyString();
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

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsMessage::commsDefOrigValidCodeInternal() const
{
    auto obj = genParseObj();

    if ((!m_customCode.m_valid.empty()) && (!hasOrigCode(obj.parseValidOverride()))) {
        return strings::genEmptyString();
    }

    auto cond = obj.parseValidCond();
    if (!cond.parseValid()) {
        return strings::genEmptyString();
    }

    auto& gen = CommsGenerator::cast(genGenerator());
    auto str = 
        CommsOptionalField::commsDslCondToString(gen, m_commsFields, cond, true);

    if (str.empty()) {
        return strings::genEmptyString();
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
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    return util::genProcessTemplate(Templ, repl);
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

    return util::genProcessTemplate(Templ, repl);
}

CommsMessage::StringsList CommsMessage::commsClientExtraCustomizationOptionsInternal() const
{
    auto sender = genParseObj().parseSender();
    if (sender == commsdsl::parse::ParseMessage::ParseSender::Both) {
        return StringsList();
    }

    if (sender == commsdsl::parse::ParseMessage::ParseSender::Client) {
        return StringsList{
            "comms::option::app::NoReadImpl",
            "comms::option::app::NoDispatchImpl"
        };
    }

    assert (sender == commsdsl::parse::ParseMessage::ParseSender::Server);
    return StringsList{
        "comms::option::app::NoWriteImpl",
        "comms::option::app::NoRefreshImpl"
    };
}

CommsMessage::StringsList CommsMessage::commsServerExtraCustomizationOptionsInternal() const
{
    auto sender = genParseObj().parseSender();
    if (sender == commsdsl::parse::ParseMessage::ParseSender::Both) {
        return StringsList();
    }

    if (sender == commsdsl::parse::ParseMessage::ParseSender::Client) {
        return StringsList{
            "comms::option::app::NoWriteImpl",
            "comms::option::app::NoRefreshImpl"
        };
    }

    assert (sender == commsdsl::parse::ParseMessage::ParseSender::Server);
    return StringsList{
        "comms::option::app::NoReadImpl",
        "comms::option::app::NoDispatchImpl"
    };
}

void CommsMessage::commsPrepareConstructCodeInternal()
{
    auto cond = genParseObj().parseConstruct();
    if (!cond.parseValid()) {
        return;
    }

    StringsList code;
    updateConstructCodeInternal(CommsGenerator::cast(genGenerator()), cond, code);

    if (code.empty()) {
        return;
    }

    m_internalConstruct = util::genStrListToString(code, "", "");
}

} // namespace commsdsl2comms
