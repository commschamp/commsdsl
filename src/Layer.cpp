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

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

void Layer::updateIncludes(Layer::IncludesList& includes) const
{
    updateIncludesImpl(includes);
}

bool Layer::prepare()
{
    return prepareImpl();
}

std::string Layer::getClassDefinition(
    const std::string& scope) const
{
    return getClassDefinitionImpl(scope);
}

Layer::Ptr Layer::create(Generator& generator, commsdsl::Layer field)
{
    using CreateFunc = std::function<Ptr (Generator& generator, commsdsl::Layer)>;
    static const CreateFunc Map[] = {
        /* Custom */ [](Generator&, commsdsl::Layer ) { return Ptr(); },
        /* Sync */ [](Generator&, commsdsl::Layer ) { return Ptr(); },
        /* Size */ [](Generator&, commsdsl::Layer ) { return Ptr(); },
        /* Id */ [](Generator&, commsdsl::Layer ) { return Ptr(); },
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

    auto str = getExtraDefaultOptionsImpl(fullScope);
    if (!str.empty()) {
        str += '\n';
    }

    return
        str +
        "/// @brief Extra options for @ref " +
        fullScope + common::nameToClassCopy(name()) + " layer.\n" +
        "using " + common::nameToClassCopy(name()) +
            " = comms::option::EmptyOption;\n";
}

std::string Layer::getPrefix() const
{
    auto str = "/// @brief Layer \"" + name() + "\".\n";
    auto& desc = m_dslObj.description();
    if (!desc.empty()) {
        str += "/// @details\n";
        auto descCpy = common::makeMultilineCopy(desc);
        common::insertIndent(descCpy);
        auto& doxyPrefix = common::doxygenPrefixStr();
        descCpy.insert(descCpy.begin(), doxyPrefix.begin(), doxyPrefix.end());
        ba::replace_all(descCpy, "\n", "\n" + doxyPrefix);
        str += descCpy;
    }
    return str;
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
