#include "Layer.h"

#include <functional>
#include <type_traits>
#include <cassert>
#include <algorithm>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "PayloadLayer.h"
#include "IdLayer.h"
#include "SizeLayer.h"
#include "SyncLayer.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

void Layer::updateIncludes(Layer::IncludesList& includes) const
{
    do {
        if (m_field) {
            m_field->updateIncludes(includes);
            break;
        }

        auto dslField = m_dslObj.field();
        if (!dslField.valid()) {
            break;
        }

        auto extRef = dslField.externalRef();
        assert(!extRef.empty());
        common::mergeInclude(m_generator.headerfileForField(extRef, false), includes);
    } while (false);

    updateIncludesImpl(includes);
}

bool Layer::prepare()
{
    auto dslField = m_dslObj.field();
    do {
        if (!dslField.valid()) {
            if (kind() != commsdsl::Layer::Kind::Payload) {
                generator().logger().error("Layer field definition is missing.");
                assert(!"Should not happen");
                return false;
            }

            break;
        }

        auto extRef = dslField.externalRef();
        if (!extRef.empty()) {
            break;
        }

        m_field = Field::create(m_generator, dslField);
    } while (false);

    return prepareImpl();
}

std::string Layer::getClassDefinition(
    const std::string& scope,
    std::string& prevLayer,
    bool& hasInputMessages) const
{
    return getClassDefinitionImpl(scope, prevLayer, hasInputMessages);
}

Layer::Ptr Layer::create(Generator& generator, commsdsl::Layer field)
{
    using CreateFunc = std::function<Ptr (Generator& generator, commsdsl::Layer)>;
    static const CreateFunc Map[] = {
        /* Custom */ [](Generator&, commsdsl::Layer ) { return Ptr(); },
        /* Sync */ [](Generator& g, commsdsl::Layer l) { return createSyncLayer(g, l); },
        /* Size */ [](Generator& g, commsdsl::Layer l) { return createSizeLayer(g, l); },
        /* Id */ [](Generator& g, commsdsl::Layer l) { return createIdLayer(g, l); },
        /* Value */ [](Generator&, commsdsl::Layer ) { return Ptr(); },
        /* Payload */ [](Generator& g, commsdsl::Layer l) { return createPayloadLayer(g, l); },
        /* Checksum */ [](Generator&, commsdsl::Layer ) { return Ptr(); }
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == (unsigned)commsdsl::Layer::Kind::NumOfValues, "Invalid map");

    auto idx = static_cast<std::size_t>(field.kind());
    if (MapSize <= idx) {
        assert(!"Unexpected layer kind");
        return Ptr();
    }

    return Map[idx](generator, field);
}

std::string Layer::getDefaultOptions(const std::string& scope) const
{
    auto fullScope = scope;
    // if (!m_externalRef.empty()) {
    //     fullScope += common::fieldStr() + "::";
    // }

    auto className = common::nameToClassCopy(name());
    std::string str;
    if (m_field) {
        static const std::string Templ = 
            "/// @brief Extra options for all the member fields of @ref #^#SCOPE#$##^#CLASS_NAME#$# layer field.\n"
            "struct #^#CLASS_NAME#$#Members\n"
            "{\n"
            "    #^#FIELD_OPT#$#\n"
            "};\n\n";

        auto fieldScope = 
            fullScope + className + 
            common::membersSuffixStr() + "::";


        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("SCOPE", fieldScope));
        replacements.insert(std::make_pair("CLASS_NAME", className));
        replacements.insert(std::make_pair("FIELD_OPT", m_field->getDefaultOptions(fieldScope)));
        str += common::processTemplate(Templ, replacements);
    }

    str += getExtraDefaultOptionsImpl(fullScope);
    // if (!str.empty()) {
    //     str += '\n';
    // }

    return
        str +
        "/// @brief Extra options for @ref " +
        fullScope + className + " layer.\n" +
        "using " + className +
        " = comms::option::EmptyOption;\n";
}

const Field* Layer::getField() const
{
    if (m_field) {
        return m_field.get();
    }

    auto extRef = m_dslObj.field().externalRef();
    assert(!extRef.empty());
    return m_generator.findField(extRef, true);
}

std::string Layer::getPrefix() const
{
    auto str = "/// @brief Definition of layer \"" + name() + "\".";
    auto& desc = m_dslObj.description();
    if (!desc.empty()) {
        str += "\n/// @details\n";
        auto descCpy = common::makeMultilineCopy(desc);
        common::insertIndent(descCpy);
        auto& doxyPrefix = common::doxygenPrefixStr();
        descCpy.insert(descCpy.begin(), doxyPrefix.begin(), doxyPrefix.end());
        ba::replace_all(descCpy, "\n", "\n" + doxyPrefix);
        str += descCpy;
    }
    return str;
}

std::string Layer::getFieldDefinition(const std::string& scope) const
{
    if (!m_field) {
        return common::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Scope for field(s) of @ref #^#CLASS_NAME#$# layer.\n"
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#FIELD_DEF#$#\n"
        "};\n";

    auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("FIELD_DEF", m_field->getClassDefinition(fullScope)));

    return common::processTemplate(Templ, replacements);
}

std::string Layer::getFieldType() const
{
    if (m_field) {
        return
            "typename " +
            common::nameToClassCopy(name()) + common::membersSuffixStr() +
            "::" + common::nameToClassCopy(m_field->name());
    }

    std::string extraOpt;
    if (m_forcedFieldFailOnInvalid) {
        extraOpt = ", comms::option::FailOnInvalid<comms::ErrorStatus::ProtocolError> ";
    }

    auto extRef = m_dslObj.field().externalRef();
    assert(!extRef.empty());
    auto* fieldPtr = m_generator.findField(extRef, true);
    static_cast<void>(fieldPtr);
    assert(fieldPtr != nullptr);
    return m_generator.scopeForField(extRef, true, true) + "<TOpt" + extraOpt + ">";
}

void Layer::setFieldForcedFailOnInvalid()
{
    m_forcedFieldFailOnInvalid = true;
    if (m_field) {
        m_field->setForcedFailOnInvalid();
    }
}

bool Layer::prepareImpl()
{
    return true;
}

// bool Layer::writeProtocolDefinition() const
// {
//     auto startInfo = m_generator.startLayerProtocolWrite(m_externalRef);
//     auto& filePath = startInfo.first;
//     if (filePath.empty()) {
//         return true;
//     }

//     assert(!m_externalRef.empty());
//     IncludesList includes;
//     updateIncludes(includes);
//     auto incStr = common::includesToStatements(includes);

//     auto namespaces = m_generator.namespacesForLayer(m_externalRef);

//     // TODO: modify class name

//     common::ReplacementMap replacements;
//     replacements.insert(std::make_pair("INCLUDES", std::move(incStr)));
//     replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
//     replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
//     replacements.insert(std::make_pair("CLASS_DEF", getClassDefinition("TOpt::" + m_generator.scopeForLayer(m_externalRef))));
//     replacements.insert(std::make_pair("FIELD_NAME", getDisplayName()));

//     std::string str = common::processTemplate(FileTemplate, replacements);

//     std::ofstream stream(filePath);
//     if (!stream) {
//         m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
//         return false;
//     }
//     stream << str;

//     if (!stream.good()) {
//         m_generator.logger().error("Failed to write \"" + filePath + "\".");
//         return false;
//     }

//     return true;
// }

void Layer::updateIncludesImpl(IncludesList& includes) const
{
    static_cast<void>(includes);
}

std::string Layer::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    static_cast<void>(scope);
    return common::emptyString();
}

} // namespace commsdsl2comms
