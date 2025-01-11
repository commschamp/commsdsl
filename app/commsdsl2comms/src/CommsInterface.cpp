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
    if (!util::isFileReadable(codePath)) {
        return;
    }

    code = util::readFileContents(codePath);
}

} // namespace 
    

CommsInterface::CommsInterface(CommsGenerator& generator, commsdsl::parse::Interface dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
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
                return fieldName == f->field().dslObj().name();
            });

    if (iter == m_commsFields.end()) {
        return nullptr;
    }

    std::string restAcc;
    if (dotPos < refStr.size()) {
        restAcc = refStr.substr(dotPos + 1);
    }

    auto info = (*iter)->field().processInnerRef(restAcc);

    if ((info.m_field != nullptr) &&
        (info.m_valueName.empty()) &&
        (info.m_refType == commsdsl::gen::Field::FieldRefType_Field)) {
        return *iter;
    }

    return nullptr;
}

bool CommsInterface::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    if (!copyCodeFromInternal()) {
        return false;
    }    

    if (dslObj().valid()) {
        m_name = comms::className(dslObj().name());
    }

    if (m_name.empty()) {
        m_name = strings::messageClassStr();
    }

    auto inputCodePrefix = comms::inputCodePathFor(*this, generator());

    readCustomCodeInternal(inputCodePrefix + strings::constructFileSuffixStr(), m_constructCode);
    readCustomCodeInternal(inputCodePrefix + strings::incFileSuffixStr(), m_customCode.m_inc);
    readCustomCodeInternal(inputCodePrefix + strings::publicFileSuffixStr(), m_customCode.m_public);
    readCustomCodeInternal(inputCodePrefix + strings::protectedFileSuffixStr(), m_customCode.m_protected);
    readCustomCodeInternal(inputCodePrefix + strings::privateFileSuffixStr(), m_customCode.m_private);
    readCustomCodeInternal(inputCodePrefix + strings::extendFileSuffixStr(), m_customCode.m_extend);
    readCustomCodeInternal(inputCodePrefix + strings::appendFileSuffixStr(), m_customCode.m_append);
    m_commsFields = CommsField::commsTransformFieldsList(fields());

    return true;
}

bool CommsInterface::writeImpl() const
{
    return 
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

bool CommsInterface::copyCodeFromInternal()
{
    auto obj = dslObj();
    if (!obj.valid()) {
        return true;
    }
    
    auto& copyFrom = obj.copyCodeFrom();
    if (copyFrom.empty()) {
        return true;
    }

    auto* origIface = generator().findInterface(copyFrom);
    if (origIface == nullptr) {
        generator().logger().error(
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

    auto& gen = generator();
    auto filePath = comms::commonHeaderPathFor(*this, gen);

    gen.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!gen.createDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        gen.logger().error("Failed to open \"" + filePath + "\" for writing.");
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

    util::ReplacementMap repl =  {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"CLASS_NAME", m_name},
        {"SCOPE", comms::scopeFor(*this, gen)},
        {"INCLUDES", commsCommonIncludesInternal()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"CODE", commsCommonFieldsCodeInternal()}
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good();
}

bool CommsInterface::commsWriteDefInternal() const
{
    auto& gen = generator();
    auto writeFunc = 
        [&gen](const std::string& filePath, const std::string& content)
        {
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

    auto obj = dslObj();
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::commsFileGeneratedComment()},
        {"CLASS_NAME", m_name},
        {"INCLUDES", commsDefIncludesInternal()},
        {"CUSTOM_INCLUDES", m_customCode.m_inc},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"DOC_DETAILS", commsDefDocDetailsInternal()},
        {"BASE", commsDefBaseClassInternal()},
        {"HEADERFILE", comms::relHeaderPathFor(*this, gen)},
        {"EXTEND", m_customCode.m_extend},
        {"APPEND", m_customCode.m_append}
    };

    if (!repl["EXTEND"].empty()) {
        repl["SUFFIX"] = strings::origSuffixStr();
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

    return writeFunc(genFilePath, util::processTemplate(*templ, repl, true));
}

std::string CommsInterface::commsCommonIncludesInternal() const
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

std::string CommsInterface::commsCommonFieldsCodeInternal() const
{
    util::StringsList fields;
    for (auto* cField : m_commsFields) {
        assert(cField != nullptr);
        auto def = cField->commsCommonCode();
        if (def.empty()) {
            continue;
        }

        fields.push_back(std::move(def));
    }

    return util::strListToString(fields, "\n", "");
}

std::string CommsInterface::commsDefIncludesInternal() const
{
    auto& gen = generator();
    util::StringsList includes = {
        "comms/Message.h",
        "comms/options.h",
        comms::relHeaderForRoot(strings::msgIdEnumNameStr(), gen),
    };

    if (!m_commsFields.empty()) {
        includes.insert(includes.end(), {
            "<tuple>",
            comms::relCommonHeaderPathFor(*this, gen)
        });
    }

    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);

        auto fIncludes = commsField->commsDefIncludes();
        includes.reserve(includes.size() + fIncludes.size());
        std::move(fIncludes.begin(), fIncludes.end(), std::back_inserter(includes));
    }
    comms::prepareIncludeStatement(includes);    
    return util::strListToString(includes, "\n", "\n");
}

std::string CommsInterface::commsDefFieldsCodeInternal() const
{
    if (m_commsFields.empty()) {
        return strings::emptyString();
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

    util::StringsList defs;
    util::StringsList names;
    for (auto* commsField : m_commsFields) {
        assert(commsField != nullptr);
        defs.push_back(commsField->commsDefCode());
        names.push_back(comms::className(commsField->field().dslObj().name()));
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", m_name},
        {"HEADERFILE", comms::relHeaderPathFor(*this, generator())},
        {"FIELDS_DEF", util::strListToString(defs, "\n", "")},
        {"FIELDS_LIST", util::strListToString(names, ",\n", "")},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsInterface::commsDefDocDetailsInternal() const
{
    std::string desc;
    if (dslObj().valid()) {
        desc = util::strMakeMultiline(dslObj().description());
    } 

    if (!desc.empty()) {
        static const std::string DocPrefix = strings::doxygenPrefixStr() + strings::incFileSuffixStr();
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + DocPrefix);
        desc = util::strReplace(desc, "\n", DocNewLineRepl);
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

    auto& gen = generator();
    auto& schema = gen.schemaOf(*this);

    util::ReplacementMap repl = {
        {"ENDIAN", comms::dslEndianToOpt(schema.schemaEndian())},
        {"MSG_ID_TYPE", comms::scopeForRoot(strings::msgIdEnumNameStr(), gen)},
        {"EXTRA_OPTS", commsDefExtraOptionsInternal()}
    };

    if (!repl["EXTRA_OPTS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsInterface::commsDefExtraOptionsInternal() const
{
    util::StringsList opts;

    if (!m_commsFields.empty()) {
        opts.push_back(
            "comms::option::def::ExtraTransportFields<" +
            m_name + 
            strings::fieldsSuffixStr() + 
            "::All>"
        );
    }
    
    auto iter =
        std::find_if(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto& f)
            {
                return f->field().dslObj().semanticType() == commsdsl::parse::Field::SemanticType::Version;
            });

    if (iter != m_commsFields.end()) {
        opts.push_back(
            "comms::option::def::VersionInExtraTransportFields<" +
            util::numToString(static_cast<std::size_t>(std::distance(m_commsFields.begin(), iter))) +
            ">"
        );
    }
    return util::strListToString(opts, ",\n", "");
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

    util::ReplacementMap repl = {
        {"CONSTRUCT", m_constructCode},
        {"ACCESS", commsDefFieldsAccessInternal()},
        {"ALIASES", commsDefFieldsAliasesInternal()},
        {"EXTRA", m_customCode.m_protected},
    };

    return util::processTemplate(Templ, repl);
}        

std::string CommsInterface::commsDefProtectedInternal() const
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

std::string CommsInterface::commsDefPrivateInternal() const
{
    if (m_customCode.m_private.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "private:\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::ReplacementMap repl = {
        {"CUSTOM", m_customCode.m_private}
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsInterface::commsDefFieldsAccessInternal() const
{
    if (m_commsFields.empty()) {
        return strings::emptyString();
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

    util::StringsList docs;
    util::StringsList names;

    auto interfaceClassName = comms::className(dslObj().name());
    for (auto& fPtr : fields()) {
        assert(fPtr);
        auto& name = fPtr->dslObj().name();
        auto accName = comms::accessName(name);
        auto className = comms::className(name);

        static const std::string DocPrefix = strings::doxygenPrefixStr() + strings::indentStr();
        auto doc = 
            DocPrefix + "@li @b TransportFieldIdx_" + accName + " index, @b TransportField_" + accName +
            " type\n" + DocPrefix + strings::indentStr() +
            "and @b transportField_" + accName + "() access fuction for @ref " + 
            interfaceClassName + strings::fieldsSuffixStr() + "::" +
            className + " field.";

        names.push_back(accName);
        docs.push_back(std::move(doc));
    }
    
    util::ReplacementMap repl = {
        {"DOC", util::strListToString(docs, "\n", "")},
        {"NAMES", util::strListToString(names, ",\n", "")},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsInterface::commsDefFieldsAliasesInternal() const
{
    auto obj = dslObj();
    if (!obj.valid()) {
        return strings::emptyString();
    }

    auto aliases = obj.aliases();
    if (aliases.empty()) {
        return strings::emptyString();    
    }

    util::StringsList result;
    for (auto& a : aliases) {
        static const std::string Templ =
            "/// @brief Alias to an extra transport member field.\n"
            "/// @details\n"
            "#^#ALIAS_DESC#$#\n"
            "///     Generates field access alias function(s):\n"
            "///     @b transportField_#^#ALIAS_NAME#$#() -> <b>transportField_#^#ALIASED_FIELD_DOC#$#</b>\n"
            "COMMS_MSG_TRANSPORT_FIELD_ALIAS(#^#ALIAS_NAME#$#, #^#ALIASED_FIELD#$#);\n";

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
            {"ALIASED_FIELD_DOC", util::strListToString(fieldSubNames, "().transportField_", "()")},
            {"ALIASED_FIELD", util::strListToString(fieldSubNames, ", ", "")}
        };

        result.push_back(util::processTemplate(Templ, repl));
    }
    
    return util::strListToString(result, "\n", "");
}

} // namespace commsdsl2comms
