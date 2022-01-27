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

        auto* commsField = dynamic_cast<const CommsField*>(fPtr.get());

        if (commsField == nullptr) {
            gen.logger().error("NYI: Class for field " + fPtr->name() + " is not implemented yet");
        }

        // TODO: uncomment assert, remove condition above
        //assert(commsField != nullptr);

        m_commsFields.push_back(commsField);
    }

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
    // TODO: implement
    return true;
}

std::string CommsMessage::commsCommonIncludesInternal() const
{
    util::StringsList includes;
    for (auto* commsField : m_commsFields) {
        // TODO: remove condition, uncomment assert
        if (commsField == nullptr) {
            continue;
        }
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
        // TODO: remove condition when all fields are defined
        if (cField == nullptr) {continue;}

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

} // namespace commsdsl2new
