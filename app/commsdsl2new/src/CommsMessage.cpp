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

#include "CommsMessage.h"

#include "CommsField.h"
#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <numeric>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2new
{

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

    m_commsFields = CommsField::commsTransformFieldsList(fields());
    m_bundledReadPrepareCodes.reserve(m_commsFields.size());
    m_bundledRefreshCodes.reserve(m_commsFields.size());
    for (auto* m : m_commsFields) {
        m_bundledReadPrepareCodes.push_back(m->commsDefBundledReadPrepareFuncBody(m_commsFields));
        m_bundledRefreshCodes.push_back(m->commsDefBundledRefreshFuncBody(m_commsFields));
    }

    m_customRead = util::readFileContents(comms::inputCodePathFor(*this, generator()) + strings::readFileSuffixStr());
    m_customRefresh = util::readFileContents(comms::inputCodePathFor(*this, generator()) + strings::refreshFileSuffixStr());
    return true;
}

bool CommsMessage::writeImpl()
{
    return 
        commsWriteCommonInternal() &&
        commsWriteDefInternal();
}

bool CommsMessage::commsWriteCommonInternal() 
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
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"SCOPE", comms::scopeFor(*this, gen)},
        {"INCLUDES", commsCommonIncludesInternal()},
        {"NS_BEGIN", comms::namespaceBeginFor(*this, gen)},
        {"NS_END", comms::namespaceEndFor(*this, gen)},
        {"FIELDS_COMMON", commsCommonFieldsCodeInternal()},
        {"NAME", comms::className(dslObj().name())},
        {"BODY", commsCommonBodyInternal()},
    };

    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

bool CommsMessage::commsWriteDefInternal()
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

    bool extended = util::isFileReadable(codePathPrefix + strings::extendFileSuffixStr());
    if (extended) {
        assert(genFilePath.size() >= 2);
        assert(genFilePath.back() == 'h');
        genFilePath.insert((genFilePath.size() - 2), strings::origSuffixStr());
    }
    
    static const std::string Templ =
        "#^#GENERATED#$#\n"
        "/// @file\n"
        "/// @brief Contains definition of <b>\"#^#MESSAGE_NAME#$#\"</b> message and its fields.\n"
        "\n"
        "#pragma once\n"
        "\n"
        "#^#INCLUDES#$#\n"
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
        "#^#NS_END#$#\n"
        "#^#APPEND#$#\n";
    
    auto obj = dslObj();
    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"MESSAGE_NAME", util::displayName(obj.displayName(), obj.name())},
        {"INCLUDES", commsDefIncludesInternal()},
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
        {"APPEND", util::readFileContents(codePathPrefix + strings::appendFileSuffixStr())}
    };

    if (extended) {
        repl["SUFFIX"] = strings::origSuffixStr();
    }

    return writeFunc(genFilePath, util::processTemplate(Templ, repl));
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
        gen.mainNamespace() + '/' + strings::msgIdEnumNameStr() + strings::cppHeaderSuffixStr(),
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
        "    comms::option::def::MsgType<#^#CLASS_NAME#$#<TMsgBase, TOpt> >,\n"
        "    comms::option::def::HasName#^#COMMA#$#\n"
        "    #^#EXTRA_OPTIONS#$#\n"
        ">";    

    auto& gen = generator();
    util::ReplacementMap repl = {
        {"CUSTOMIZATION_OPT", commsDefCustomizationOptInternal()},
        {"MESSAGE_ID", comms::messageIdStrFor(*this, gen)},
        {"CLASS_NAME", comms::className(dslObj().name())},
        {"EXTRA_OPTIONS", commsDefExtraOptionsInternal()},
    };

    if (!repl["EXTRA_OPTIONS"].empty()) {
        repl["COMMA"] = ",";
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefCustomizationOptInternal() const
{
    std::string result;
    if (commsIsCustomizableInternal()) {
        result = "typename TOpt::" + comms::scopeFor(*this, generator(), false, true) + ",";
    }
    return result;
}

std::string CommsMessage::commsDefExtraOptionsInternal() const
{
    util::StringsList opts;
    bool hasGeneratedRead = 
        std::any_of(
            m_bundledReadPrepareCodes.begin(), m_bundledReadPrepareCodes.end(),
            [](const std::string& code)
            {
                return !code.empty();
            });

    bool hasGeneratedRefresh = 
        std::any_of(
            m_bundledRefreshCodes.begin(), m_bundledRefreshCodes.end(),
            [](const std::string& code)
            {
                return !code.empty();
            });

    if ((!m_customRead.empty()) || hasGeneratedRead) {
        util::addToStrList("comms::option::def::HasCustomRead", opts);
    }

    if ((!m_customRefresh.empty()) || hasGeneratedRefresh) {
        return "comms::option::def::HasCustomRefresh";
    }

    return strings::emptyString();    
}

std::string CommsMessage::commsDefPublicInternal() const
{
    static const std::string Templ =
        "public:\n"
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
        {"ACCESS", commsDefFieldsAccessInternal()},
        {"ALIASES", commsDefFieldsAliasesInternal()},
        {"LENGTH_CHECK", commsDefLengthCheckInternal()},
        {"EXTRA", util::readFileContents(inputCodePrefix + strings::publicFileSuffixStr())},
        {"NAME", commsDefNameFuncInternal()},
        {"READ", commsDefReadFuncInternal()},
        {"WRITE", util::readFileContents(inputCodePrefix + strings::writeFileSuffixStr())},
        {"LENGTH", util::readFileContents(inputCodePrefix + strings::lengthFileSuffixStr())},
        {"VALID", util::readFileContents(inputCodePrefix + strings::validFileSuffixStr())},
        {"REFRESH", commsDefRefreshFuncInternal()},
    };

    return util::processTemplate(Templ, repl);
}        

std::string CommsMessage::commsDefProtectedInternal() const
{
    auto custom = util::readFileContents(comms::inputCodePathFor(*this, generator()) + strings::protectedFileSuffixStr());
    if (custom.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "protected:\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::ReplacementMap repl = {
        {"CUSTOM", std::move(custom)}
    };
    
    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefPrivateInternal() const
{
    auto custom = util::readFileContents(comms::inputCodePathFor(*this, generator()) + strings::privateFileSuffixStr());

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

    if (reads.empty() && refreshes.empty() && custom.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "private:\n"
        "    #^#READS#$#\n"
        "    #^#REFRESHES#$#\n"
        "    #^#CUSTOM#$#\n"
    ;

    util::ReplacementMap repl = {
        {"READS", util::strListToString(reads, "\n", "")},
        {"REFRESHES", util::strListToString(refreshes, "\n", "")},
        {"CUSTOM", std::move(custom)}
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

    for (auto& fPtr : fields()) {
        assert(fPtr);
        auto& name = fPtr->dslObj().name();
        auto accName = comms::accessName(name);
        auto className = comms::className(name);

        static const std::string DocPrefix = strings::doxygenPrefixStr() + strings::indentStr();
        auto doc = 
            DocPrefix + "@li @b FieldIdx_" + accName + " index, @b Field_" + accName +
            " type and @b field_" + accName + "() access fuction\n" +
            DocPrefix + strings::indentStr() + "for @ref " + className + " field.";

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
        auto fieldSubNames = util::strSplitByAnyCharCompressed(fieldName, ".");
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
    static const std::string Templ = 
        "// Compile time check for serialisation length.\n"
        "static const std::size_t MsgMinLen = Base::doMinLength();\n"
        "#^#MAX_LEN#$#\n"
        "static_assert(MsgMinLen == #^#MIN_LEN_VAL#$#, \"Unexpected min serialisation length\");\n"
        "#^#MAX_LEN_ASSERT#$#\n"
    ;

    auto& fList = fields();
    auto minLength =
        std::accumulate(
            fList.begin(), fList.end(), std::size_t(0),
            [](std::size_t soFar, auto& f)
            {
                return comms::addLength(soFar, f->dslObj().minLength());
            });
    auto maxLength =
        std::accumulate(
            fList.begin(), fList.end(), std::size_t(0),
            [](std::size_t soFar, auto& f)
            {
                return comms::addLength(soFar, f->dslObj().maxLength());
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
    static const std::string Templ = 
        "/// @brief Name of the message.\n"
        "static const char* doName#^#ORIG#$#()\n"
        "{\n"
        "    return #^#SCOPE#$#::name();\n"
        "}\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl = {
        {"SCOPE", comms::commonScopeFor(*this, generator())},
        {"CUSTOM", util::readFileContents(comms::inputCodePathFor(*this, generator()) + strings::nameFileSuffixStr())},
    };

    if (!repl["CUSTOM"].empty()) {
        repl.insert({
            {"ORIG", strings::origSuffixStr()}
        });
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefReadFuncInternal() const
{
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

    if (reads.empty()) {
        // Members dont have bundled reads
        return strings::emptyString();    
    }

    if (prevIdx < 0) {
        // Only the first element has readPrepare()
        reads.push_back("es = Base::doRead(iter, len);\n");
    }
    else {
        auto prevAcc = comms::accessName(m_commsFields[prevIdx]->field().dslObj().name());
        reads.push_back("es = Base::teamplate doReadFrom<FieldIdx_" + prevAcc + ">(iter, len);\n");
    }

    if (reads.empty()) {
        return m_customRead;
    }

    static const std::string Templ = 
        "/// @brief Generated read functionality.\n"
        "template <typename TIter>\n"
        "comms::ErrorStatus doRead#^#ORIG#$#(TIter& iter, std::size_t len)\n"
       "{\n"
       "    #^#UPDATE_VERSION#$#\n"
       "    #^#READS#$#\n"
       "}\n"
       "#^#CUSTOM#$#\n"
    ;
    
    util::ReplacementMap repl = {
        {"READS", util::strListToString(reads, "\n", "")},
        {"CUSTOM", m_customRead},
        {"UPDATE_VERSION", generator().versionDependentCode() ? "Base::doFieldsVersionUpdate();" : strings::emptyString()},
    };

    if (!m_customRead.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    return util::processTemplate(Templ, repl);
}

std::string CommsMessage::commsDefRefreshFuncInternal() const
{
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
        return m_customRefresh;
    }
    
    static const std::string Templ = 
        "/// @brief Generated refresh functionality.\n"
        "bool doRefresh#^#ORIG#$#()\n"
        "{\n"
        "   bool updated = Base::doRefresh();\n"
        "   #^#FIELDS#$#\n"
        "   return updated;"
        "}\n"
        "#^#CUSTOM#$#\n";

    util::ReplacementMap repl = {
        {"FIELDS", util::strListToString(fields, "\n", "")},
        {"CUSTOM", m_customRefresh}
    };

    if (!m_customRefresh.empty()) {
        repl["ORIG"] = strings::origSuffixStr();
    }

    return util::processTemplate(Templ, repl);
}

bool CommsMessage::commsIsCustomizableInternal() const
{
    auto& gen = static_cast<const CommsGenerator&>(generator());
    auto level = gen.getCustomizationLevel();
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

// bool CommsMessage::commsMustGenerateReadRefresh() const
// {
//     return 
//         std::any_of(
//             m_commsFields.begin(), m_commsFields.end(),
//             [](auto* f)
//             {
//                 return f->doesRequireGeneratedReadRefresh();
//             });
// }

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
            repl["EXT"] = " : public TBase::" + comms::scopeFor(*this, generator(), false) + strings::fieldsSuffixStr();
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
            extraOpts.push_back("comms::options::EmptyOption");
        }

        if ((!extraOpts.empty()) && (hasBase)) {
            extraOpts.push_back("typename TBase::" + comms::scopeFor(*this, generator(), false));
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

} // namespace commsdsl2new
