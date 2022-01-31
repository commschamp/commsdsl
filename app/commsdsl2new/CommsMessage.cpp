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

bool CommsMessage::prepareImpl()
{
    auto& gen = generator();
    auto& genFields = fields();
    m_commsFields.reserve(genFields.size());
    for (auto& fPtr : genFields) {
        assert(fPtr);

        auto* commsField = 
            const_cast<CommsField*>(
                dynamic_cast<const CommsField*>(fPtr.get()));

        // TODO: remove this condition
        if (commsField == nullptr) {
            gen.logger().error("NYI: Class for field " + fPtr->name() + " is not implemented yet");
            continue;
        }

        assert(commsField != nullptr);
        commsField->setReferenced();
        m_commsFields.push_back(commsField);
    }

    m_customRefresh = util::readFileContents(comms::inputCodePathFor(*this, gen) + strings::refreshFileSuffixStr());
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
    if ((!m_customRefresh.empty()) || (commsMustGenerateReadRefresh())) {
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

    util::ReplacementMap repl = {
        {"ACCESS", commsDefFieldsAccess()},
        {"ALIASES", commsDefFieldsAliases()},

        // TODO: incomplete
    };

    return util::processTemplate(Templ, repl);
}        

std::string CommsMessage::commsDefProtectedInternal() const
{
    // TODO:
    return strings::emptyString();
}

std::string CommsMessage::commsDefPrivateInternal() const
{
    // TODO:
    return strings::emptyString();
}

std::string CommsMessage::commsDefFieldsAccess() const
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

std::string CommsMessage::commsDefFieldsAliases() const
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

bool CommsMessage::commsMustGenerateReadRefresh() const
{
    return 
        std::any_of(
            m_commsFields.begin(), m_commsFields.end(),
            [](auto* f)
            {
                return f->hasGeneratedReadRefresh();
            });
}

} // namespace commsdsl2new
