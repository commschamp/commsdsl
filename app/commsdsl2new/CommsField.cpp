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

    return 
        commsWriteCommon() &&
        commsWriteDef();
}

CommsField::IncludesList CommsField::commsCommonIncludes() const
{
    return commsCommonIncludesImpl();
}

CommsField::IncludesList CommsField::commsCommonIncludesImpl() const
{
    return IncludesList();
}

bool CommsField::commsWriteCommon() const
{
    auto& generator = m_field.generator();
    auto filePath = generator.getOutputDir() + '/' + commsdsl::gen::comms::relHeaderPathFor(m_field, generator);

    assert(2 <= filePath.size());
    assert(filePath.back() == 'h');
    filePath.insert(filePath.size() - strings::cppHeaderSuffixStr().size(), strings::commonSuffixStr());

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
        "#^#INCLUDES#$#\n"
        "#^#NS_BEGIN#$#\n\n"
        "#^#NS_END#$#"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", CommsGenerator::fileGeneratedComment()},
        {"FIELD_SCOPE", comms::scopeFor(m_field, generator)},
        {"INCLUDES", util::strListToString(includes, "\n", "\n")},
        {"NS_BEGIN", comms::namespaceBeginFor(m_field, generator)},
        {"NS_END", comms::namespaceEndFor(m_field, generator)},
    };
    
    stream << util::processTemplate(Templ, repl);
    stream.flush();
    return stream.good();
}

bool CommsField::commsWriteDef() const
{
    return true;
}

} // namespace commsdsl2new
