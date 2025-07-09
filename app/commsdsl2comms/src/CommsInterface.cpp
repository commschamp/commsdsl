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

#include "CommsInterface.h"

#include "CommsField.h"
#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

const std::string& aliasTempl()
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$##^#SUFFIX#$#\"</b> interface class.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#CUSTOM_INCLUDES#$#\n\n"
        "#^#NS_BEGIN#$#\n"
        "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"</b> common interface class.\n"
        "#^#DOC_DETAILS#$#\n"
        "/// @tparam TOpt Interface definition options\n"
        "/// @headerfile #^#HEADERFILE#$#\n"
        "template <typename... TOpt>\n"
        "using #^#CLASS_NAME#$##^#SUFFIX#$# =\n"
        "    #^#BASE#$#;\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "#^#NS_END#$#\n";

    return Templ;
}

const std::string& classTempl()
{
    static const std::string Templ = 
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$#\"</b> interface class.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#CUSTOM_INCLUDES#$#\n\n"
        "#^#NS_BEGIN#$#\n"
        "#^#FIELDS_DEF#$#\n"
        "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"</b> common interface class.\n"
        "#^#DOC_DETAILS#$#\n"
        "/// @tparam TOpt Interface definition options\n"
        "/// @headerfile #^#HEADERFILE#$#\n"
        "template <typename... TOpt>\n"
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public\n"
        "    #^#BASE#$#\n"
        "{\n"
        "    using Base =\n"
        "        #^#BASE#$#;\n"
        "#^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n\n"
        "#^#EXTEND#$#\n"
        "#^#APPEND#$#\n"
        "#^#NS_END#$#\n";

    return Templ;
}

void readCustomCodeInternal(const std::string& codePath, std::string& code)
{
    if (!util::genIsFileReadable(codePath)) {
        return;
    }

    code = util::genReadFileContents(codePath);
}

} // namespace 
    

CommsInterface::CommsInterface(CommsGenerator& generator, commsdsl::parse::ParseInterface parseObj, commsdsl::gen::GenElem* parent) :
    Base(generator, parseObj, parent)
{
}   

CommsInterface::~CommsInterface() = default;

const CommsField* CommsInterface::findValidReferencedField(const std::string& refStr) const
{
    auto dotPos = refStr.find(".");
    std::string fieldName(refStr, 0, dotPos);

    auto iter = 
        std::find_if(
            m_commsFields.begin(), m_commsFields.end(),
            [&fieldName](auto* f)
            {
                return fieldName == f->commsGenField().genParseObj().parseName();
            });

    if (iter == m_commsFields.end()) {
        return nullptr;
    }

    std::string restAcc;
    if (dotPos < refStr.size()) {
        restAcc = refStr.substr(dotPos + 1);
    }

    auto info = (*iter)->commsGenField().genProcessInnerRef(restAcc);

    if ((info.m_field != nullptr) &&
        (info.m_valueName.empty()) &&
        (info.m_refType == commsdsl::gen::GenField::FieldRefType_Field)) {
        return *iter;
    }

    return nullptr;
}

bool CommsInterface::genPrepareImpl()
{
    if (!Base::genPrepareImpl()) {
        return false;
    }

    if (!copyCodeFromInternal()) {
        return false;
    }    

    if (genParseObj().parseValid()) {
        m_name = comms::genClassName(genParseObj().parseName());
    }

    if (m_name.empty()) {
        m_name = strings::genMessageClassStr();
    }

    auto inputCodePrefix = comms::genInputCodePathFor(*this, genGenerator());

    readCustomCodeInternal(inputCodePrefix + strings::genConstructFileSuffixStr(), m_constructCode);
    readCustomCodeInternal(inputCodePrefix + strings::genIncFileSuffixStr(), m_customCode.m_inc);
    readCustomCodeInternal(inputCodePrefix + strings::genPublicFileSuffixStr(), m_customCode.m_public);
    readCustomCodeInternal(inputCodePrefix + strings::genProtectedFileSuffixStr(), m_customCode.m_protected);
    readCustomCodeInternal(inputCodePrefix + strings::genPrivateFileSuffixStr(), m_customCode.m_private);
    readCustomCodeInternal(inputCodePrefix + strings::genExtendFileSuffixStr(), m_customCode.m_extend);
    readCustomCodeInternal(inputCodePrefix + strings::genAppendFileSuffixStr(), m_customCode.m_append);
    m_commsFields = CommsField::commsTransformFieldsList(genFields());

    return true;
}

bool CommsInterface::genWriteImpl() const
{
    return 
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

bool CommsInterface::copyCodeFromInternal()
{
    auto obj = genParseObj();
    if (!obj.parseValid()) {
        return true;
    }
    
    auto& copyFrom = obj.parseCopyCodeFrom();
    if (copyFrom.empty()) {
        return true;
    }

    auto* origIface = genGenerator().genFindInterface(copyFrom);
    if (origIface == nullptr) {
        genGenerator().genLogger().genError(
            "Failed to find referenced field \"" + copyFrom + "\" for copying overriding code.");
        assert(false); // Should not happen
        return false;
    }

    auto* commsIface = dynamic_cast<const CommsInterface*>(origIface);
    assert(commsIface != nullptr);
    m_customCode = commsIface->m_customCode;
    return true;
}

bool CommsInterface::commsWriteCommonInternal() const
{
    if (m_commsFields.empty()) {
        return true;
    }

    auto& gen = genGenerator();
    auto filePath = comms::genCommonHeaderPathFor(*this, gen);

    gen.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        gen.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }    

    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains common template parameters independent functionality of\n"
        "///    @ref #^#SCOPE#$# interface fields.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n"
        "/// @brief Common types and functions for fields of \n"
        "///     @ref #^#SCOPE#$# interface.\n"
        "/// @see #^#SCOPE#$#Fields\n"
        "struct #^#CLASS_NAME#$#FieldsCommon\n"
        "{\n"
        "    #^#CODE#$#\n"
        "};\n"
        "#^#NS_END#$#\n";

    util::GenReplacementMap repl =  {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"CLASS_NAME", m_name},
        {"SCOPE", comms::genScopeFor(*this, gen)},
        {"INCLUDES", commsCommonIncludesInternal()},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"CODE", commsCommonFieldsCodeInternal()}
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool CommsInterface::commsWriteDefInternal() const
{
    auto& gen = genGenerator();
    auto writeFunc = 
        [&gen](const std::string& filePath, const std::string& content)
        {
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

    auto obj = genParseObj();
    util::GenReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"CLASS_NAME", m_name},
        {"INCLUDES", commsDefIncludesInternal()},
        {"CUSTOM_INCLUDES", m_customCode.m_inc},
        {"NS_BEGIN", comms::genNamespaceBeginFor(*this, gen)},
        {"NS_END", comms::genNamespaceEndFor(*this, gen)},
        {"DOC_DETAILS", commsDefDocDetailsInternal()},
        {"BASE", commsDefBaseClassInternal()},
        {"HEADERFILE", comms::genRelHeaderPathFor(*this, gen)},
        {"EXTEND", m_customCode.m_extend},
        {"APPEND", m_customCode.m_append}
    };

    if (!repl["EXTEND"].empty()) {
        repl["SUFFIX"] = strings::genOrigSuffixStr();
    }

    bool useClass = 
        (!m_commsFields.empty()) ||
        (!m_constructCode.empty()) ||
        (!m_customCode.m_public.empty()) ||
        (!m_customCode.m_protected.empty()) ||
        (!m_customCode.m_private.empty());

    const std::string* templ = &(aliasTempl());
    if (useClass) {
        repl.insert({
            {"FIELDS_DEF", commsDefFieldsCodeInternal()},
            {"PUBLIC", commsDefPublicInternal()},
            {"PROTECTED", commsDefProtectedInternal()},
            {"PRIVATE", commsDefPrivateInternal()},
        });
        templ = &(classTempl());
    }

    return writeFunc(genFilePath, util::genProcessTemplate(*templ, repl, true));
}

std::string CommsInterface::commsCommonIncludesInternal() const
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

std::string CommsInterface::commsCommonFieldsCodeInternal() const
{
    util::GenStringsList fields;
    for (auto* cField : m_commsFields) {
        assert(cField != nullptr);
        auto def = cField->commsCommonCode();
        if (def.empty()) {
            continue;
        }

        fields.push_back(std::move(def));
    }

    return util::genStrListToString(fields, "\n", "");
}

std::string CommsInterface::commsDefIncludesInternal() const
{
    auto& gen = genGenerator();
    util::GenStringsList includes = {
        "comms/Message.h",
        "comms/options.h",
        comms::genRelHeaderForMsgId(strings::genMsgIdEnumNameStr(), gen, *static_cast<const commsdsl::gen::GenNamespace*>(genGetParent())),
    };

    if (!m_commsFields.empty()) {
        includes.insert(includes.end(), {
            "<tuple>",
            comms::genRelCommonHeaderPathFor(*this, gen)
        });
    }

    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);

        auto fIncludes = commsField->commsDefIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }
    comms::genPrepareIncludeStatement(includes);    
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CommsInterface::commsDefFieldsCodeInternal() const
{
    if (m_commsFields.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "/// @brief Extra transport fields of @ref #^#CLASS_NAME#$# interface class.\n"
        "/// @see @ref #^#CLASS_NAME#$#\n"
        "/// @headerfile #^#HEADERFILE#$#\n"
        "struct #^#CLASS_NAME#$#Fields\n"
        "{\n"
        "    #^#FIELDS_DEF#$#\n"
        "    /// @brief All the fields bundled in std::tuple.\n"
        "    using All = std::tuple<\n"
        "        #^#FIELDS_LIST#$#\n"
        "    >;\n"
        "};\n";

    util::GenStringsList defs;
    util::GenStringsList names;
    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);
        defs.push_back(commsField->commsDefCode());
        names.push_back(comms::genClassName(commsField->commsGenField().genParseObj().parseName()));
    }

    util::GenReplacementMap repl = {
        {"CLASS_NAME", m_name},
        {"HEADERFILE", comms::genRelHeaderPathFor(*this, genGenerator())},
        {"FIELDS_DEF", util::genStrListToString(defs, "\n", "")},
        {"FIELDS_LIST", util::genStrListToString(names, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsInterface::commsDefDocDetailsInternal() const
{
    std::string desc;
    if (genParseObj().parseValid()) {
        desc = util::genStrMakeMultiline(genParseObj().parseDescription());
    } 

    if (!desc.empty()) {
        static const std::string DocPrefix = strings::genDoxygenPrefixStr() + strings::genIncFileSuffixStr();
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + DocPrefix);
        desc = util::genStrReplace(desc, "\n", DocNewLineRepl);
        desc += " @n";
    }
    return desc;    
}

std::string CommsInterface::commsDefBaseClassInternal() const
{
    static const std::string Templ = 
        "comms::Message<\n"
        "    TOpt...,\n"
        "    #^#ENDIAN#$#,\n"
        "    comms::option::def::MsgIdType<#^#MSG_ID_TYPE#$#>#^#COMMA#$#\n"
        "    #^#EXTRA_OPTS#$#\n"
        ">";

    auto& gen = genGenerator();
    auto& schema = gen.genSchemaOf(*this);

    assert(genGetParent() != nullptr);
    assert(genGetParent()->genElemType() == commsdsl::gen::GenElem::Type_Namespace);
    util::GenReplacementMap repl = {
        {"ENDIAN", comms::genParseEndianToOpt(schema.genSchemaEndian())},
        {"MSG_ID_TYPE", comms::genScopeForMsgId(strings::genMsgIdEnumNameStr(), gen, *static_cast<const commsdsl::gen::GenNamespace*>(genGetParent()))},
        {"EXTRA_OPTS", commsDefExtraOptionsInternal()}
    };

    if (!repl["EXTRA_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsInterface::commsDefExtraOptionsInternal() const
{
    util::GenStringsList opts;

    if (!m_commsFields.empty()) {
        opts.push_back(
            "comms::option::def::ExtraTransportFields<" +
            m_name + 
            strings::genFieldsSuffixStr() + 
            "::All>"
        );
    }
    
    auto iter =
        std::find_if(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto& f)
            {
                return f->commsGenField().genParseObj().parseSemanticType() == commsdsl::parse::ParseField::ParseSemanticType::Version;
            });

    if (iter != m_commsFields.end()) {
        opts.push_back(
            "comms::option::def::VersionInExtraTransportFields<" +
            util::genNumToString(static_cast<std::size_t>(std::distance(m_commsFields.begin(), iter))) +
            ">"
        );
    }
    return util::genStrListToString(opts, ",\n", "");
}

std::string CommsInterface::commsDefPublicInternal() const
{
    static const std::string Templ =
        "public:\n"
        "    #^#CONSTRUCT#$#\n"
        "    #^#ACCESS#$#\n"
        "    #^#ALIASES#$#\n"
        "    #^#EXTRA#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"CONSTRUCT", m_constructCode},
        {"ACCESS", commsDefFieldsAccessInternal()},
        {"ALIASES", commsDefFieldsAliasesInternal()},
        {"EXTRA", m_customCode.m_public},
    };

    return util::genProcessTemplate(Templ, repl);
}        

std::string CommsInterface::commsDefProtectedInternal() const
{
    if (m_customCode.m_protected.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "protected:\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"CUSTOM", m_customCode.m_protected}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsInterface::commsDefPrivateInternal() const
{
    if (m_customCode.m_private.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "private:\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"CUSTOM", m_customCode.m_private}
    };
    
    return util::genProcessTemplate(Templ, repl);
}

std::string CommsInterface::commsDefFieldsAccessInternal() const
{
    if (m_commsFields.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "/// @brief Allow access to extra transport fields.\n"
        "/// @details See definition of @b COMMS_MSG_TRANSPORT_FIELDS_NAMES macro\n"
        "///     related to @b comms::Message class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///     The generated values, types and functions are:\n"
        "#^#DOC#$#\n"
        "COMMS_MSG_TRANSPORT_FIELDS_NAMES(\n"
        "    #^#NAMES#$#\n"
        ");\n"
        ;

    util::GenStringsList docs;
    util::GenStringsList names;

    auto interfaceClassName = comms::genClassName(genParseObj().parseName());
    for (auto& fPtr : genFields()) {
        assert(fPtr);
        auto& name = fPtr->genParseObj().parseName();
        auto accName = comms::genAccessName(name);
        auto className = comms::genClassName(name);

        static const std::string DocPrefix = strings::genDoxygenPrefixStr() + strings::genIndentStr();
        auto doc = 
            DocPrefix + "@li @b TransportFieldIdx_" + accName + " index, @b TransportField_" + accName +
            " type\n" + DocPrefix + strings::genIndentStr() +
            "and @b transportField_" + accName + "() access fuction for @ref " + 
            interfaceClassName + strings::genFieldsSuffixStr() + "::" +
            className + " field.";

        names.push_back(accName);
        docs.push_back(std::move(doc));
    }
    
    util::GenReplacementMap repl = {
        {"DOC", util::genStrListToString(docs, "\n", "")},
        {"NAMES", util::genStrListToString(names, ",\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CommsInterface::commsDefFieldsAliasesInternal() const
{
    auto obj = genParseObj();
    if (!obj.parseValid()) {
        return strings::genEmptyString();
    }

    auto aliases = obj.parseAliases();
    if (aliases.empty()) {
        return strings::genEmptyString();    
    }

    util::GenStringsList result;
    for (auto& a : aliases) {
        static const std::string Templ =
            "/// @brief Alias to an extra transport member field.\n"
            "/// @details\n"
            "#^#ALIAS_DESC#$#\n"
            "///     Generates field access alias function(s):\n"
            "///     @b transportField_#^#ALIAS_NAME#$#() -> <b>transportField_#^#ALIASED_FIELD_DOC#$#</b>\n"
            "COMMS_MSG_TRANSPORT_FIELD_ALIAS(#^#ALIAS_NAME#$#, #^#ALIASED_FIELD#$#);\n";

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

        util::GenReplacementMap repl = {
            {"ALIAS_DESC", std::move(desc)},
            {"ALIAS_NAME", comms::genAccessName(a.parseName())},
            {"ALIASED_FIELD_DOC", util::genStrListToString(fieldSubNames, "().transportField_", "()")},
            {"ALIASED_FIELD", util::genStrListToString(fieldSubNames, ", ", "")}
        };

        result.push_back(util::genProcessTemplate(Templ, repl));
    }
    
    return util::genStrListToString(result, "\n", "");
}

} // namespace commsdsl2comms
