#include "CommsField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;

namespace commsdsl2new
{

namespace 
{

void convertToCommonIncludePath(std::string& filePath)
{
    assert(2 <= filePath.size());
    assert(filePath.back() == 'h');
    filePath.insert(filePath.size() - strings::cppHeaderSuffixStr().size(), strings::commonSuffixStr());
}

} // namespace 
    

CommsField::CommsField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

CommsField::~CommsField() = default;

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
    if (!dslObj.isForceGen()) {
        // TODO: check field has been accessed
    }

    return 
        commsWriteCommon() &&
        commsWriteDef();
}

CommsField::IncludesList CommsField::commsCommonIncludes() const
{
    return commsCommonIncludesImpl();
}


std::string CommsField::commsCommonCode() const
{
    static const std::string Templ = 
        "/// @brief Common types and functions for\n"
        "///     @ref #^#SCOPE#$# field.\n"
        "struct #^#NAME#$#Common\n"
        "{\n"
        "    #^#BODY#$#\n"
        "};\n"
    ;

    auto& generator = m_field.generator();
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(m_field, generator)},
        {"NAME", comms::className(m_field.name())},
        {"BODY", commsCommonCodeBodyImpl()},
    };

    return util::processTemplate(Templ, repl);
}

CommsField::IncludesList CommsField::commsDefIncludes() const
{
    auto& generator = m_field.generator();

    IncludesList list = {
        "comms/options.h",
        comms::relHeaderPathForField(strings::fieldBaseClassStr(), generator),
    };

    if (comms::isGlobalField(m_field)) {
        auto relHeader = comms::relHeaderPathFor(m_field, generator);
        convertToCommonIncludePath(relHeader);
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
    return commsDefCodeImpl();
}

CommsField::IncludesList CommsField::commsCommonIncludesImpl() const
{
    return IncludesList();
}

std::string CommsField::commsCommonCodeBodyImpl() const
{
    return strings::emptyString();
}

CommsField::IncludesList CommsField::commsDefIncludesImpl() const
{
    return IncludesList();
}

std::string CommsField::commsDefCodeImpl() const
{
    return strings::emptyString();
}

std::string CommsField::commsCommonNameFuncCode() const
{
    auto& generator = m_field.generator();
    auto customNamePath = 
        generator.getCodeDir() + '/' + strings::includeDirStr() + '/' + 
        comms::relHeaderPathFor(m_field, generator) + strings::nameFileSuffixStr();

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

bool CommsField::commsIsVersionOptional() const
{
    auto& generator = m_field.generator();
    if (!generator.versionDependentCode()) {
        return false;
    }

    auto& dslObj = m_field.dslObj();
    if (!generator.isElementOptional(dslObj.sinceVersion(), dslObj.deprecatedSince(), dslObj.isDeprecatedRemoved())) {
        return false;
    }

    auto* parent = m_field.getParent();
    assert(parent != nullptr);
    if (comms::sinceVersionOf(*parent) < dslObj.sinceVersion()) {
        return true;
    }
    
    if ((dslObj.deprecatedSince() < commsdsl::parse::Protocol::notYetDeprecated()) &&
        (dslObj.isDeprecatedRemoved())) {
        return true;
    }

    return false;    
}

bool CommsField::commsWriteCommon() const
{
    auto& generator = m_field.generator();
    auto filePath = generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + commsdsl::gen::comms::relHeaderPathFor(m_field, generator);
    convertToCommonIncludePath(filePath);

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

bool CommsField::commsWriteDef() const
{
    auto& generator = m_field.generator();
    auto filePath = generator.getOutputDir() + '/' + strings::includeDirStr() + '/' + commsdsl::gen::comms::relHeaderPathFor(m_field, generator);

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
        "#^#NS_BEGIN#$#\n"
        "#^#DEF#$#\n"
        "#^#NS_END#$#"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"FIELD_NAME", util::displayName(m_field.dslObj().displayName(), m_field.dslObj().name())},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
        {"DEF", commsDefCode()},
    };
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

} // namespace commsdsl2new
