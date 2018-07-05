#include "Frame.h"

#include <cassert>
#include <fstream>
#include <map>
#include <algorithm>
#include <iterator>
#include <numeric>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string Template(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#CLASS_NAME#$#\"<\\b> frame class.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "/// @brief Layers definition of @ref #^#CLASS_NAME#$# frame class.\n"
    "/// @tparam TOpt Protocol options.\n"
    "/// @see @ref #^#CLASS_NAME#$#\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "template <typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions>\n"
    "struct #^#CLASS_NAME#$#Layers\n"
    "{\n"
    "    #^#LAYERS_DEF#$#\n"
    "};\n\n"
    "/// @brief Definition of <b>\"#^#CLASS_NAME#$#\"<\\b> frame class.\n"
    "#^#DOC_DETAILS#$#\n"
    "/// @tparam TOpt Frame definition options\n"
    "/// @headerfile #^#HEADERFILE#$#\n"
    "template <\n"
    "   typename TMessage,\n"
    "   #^#INPUT_MESSAGES#$#\n"
    "   typename TOpt = #^#PROT_NAMESPACE#$#::DefaultOptions\n"
    ">\n"
    "class #^#CLASS_NAME#$# : public\n"
    "    #^#FRAME_DEF#$#\n"
    "{\n"
    "    using Base =\n"
    "        #^#FRAME_DEF#$#;\n"
    "public:\n"
    "    /// @brief Allow access to frame definition layers.\n"
    "    /// @details See definition of @b COMMS_PROTOCOL_LAYERS_ACCESS macro\n"
    "    ///     from COMMS library for details.\n"
    "    ///\n"
    "    ///     The generated functions are:\n"
    "    #^#ACCESS_FUNCS_DOC#$#\n"
    "    COMMS_PROTOCOL_LAYERS_ACCESS(\n"
    "        #^#LAYERS_ACCESS_LIST#$#\n"
    "    );\n"
    "};\n\n"
    "#^#END_NAMESPACE#$#\n"
);

} // namespace

bool Frame::prepare()
{
    if (!m_dslObj.valid()) {
        assert(!"NYI");
        return true;
    }


    m_externalRef = m_dslObj.externalRef();
    if (m_externalRef.empty()) {
        m_generator.logger().log(commsdsl::ErrorLevel_Error, "Unknown external reference for frame: " + m_dslObj.name());
        return false;
    }

    auto dslLayers = m_dslObj.layers();
    m_layers.reserve(dslLayers.size());
    for (auto& l : dslLayers) {
        auto ptr = Layer::create(m_generator, l);
        assert(ptr);
        if (!ptr->prepare()) {
            return false;
        }
        m_layers.push_back(std::move(ptr));
    }

    // TODO: re-arange
    return true;
}

bool Frame::write()
{
    // TODO: write plugin
    return writeProtocol();
}

std::string Frame::getDefaultOptions() const
{
    common::StringsList layersOpts;
    layersOpts.reserve(m_layers.size());
    auto scope = m_generator.scopeForFrame(m_externalRef, true, true) + common::layersSuffixStr() + "::";
    for (auto iter = m_layers.rbegin(); iter != m_layers.rend(); ++iter) {
        layersOpts.push_back((*iter)->getDefaultOptions(scope));
    }

    static const std::string Templ = 
        "/// @brief Extra options for Layers of @ref #^#FRAME_SCOPE#$# frame.\n"
        "struct #^#CLASS_NAME#$#Layers\n"
        "{\n"
        "    #^#LAYERS_OPTS#$#\n"
        "}; // struct #^#CLASS_NAME#$#Layers\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(m_dslObj.name())));
    replacements.insert(std::make_pair("FRAME_SCOPE", m_generator.scopeForFrame(externalRef(), true, true)));
    replacements.insert(std::make_pair("LAYERS_OPTS", common::listToString(layersOpts, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

bool Frame::writeProtocol()
{
    auto names =
        m_generator.startFrameProtocolWrite(m_externalRef);
    auto& filePath = names.first;
    auto& className = names.second;

    if (filePath.empty()) {
        // Skipping generation
        return true;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", className));
    replacements.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("DOC_DETAILS", getDescription()));
    replacements.insert(std::make_pair("INCLUDES", getIncludes()));
    replacements.insert(std::make_pair("HEADERFILE", m_generator.headerfileForFrame(m_externalRef)));
    replacements.insert(std::make_pair("LAYERS_DEF", getLayersDef()));

    auto namespaces = m_generator.namespacesForFrame(m_externalRef);
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));

    auto str = common::processTemplate(Template, replacements);

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }
    stream << str;

    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Frame::getDescription() const
{
    if (!m_dslObj.valid()) {
        return common::emptyString();
    }

    auto desc = common::makeMultilineCopy(m_dslObj.description());
    if (!desc.empty()) {
        static const std::string DocPrefix("/// @details ");
        desc.insert(desc.begin(), DocPrefix.begin(), DocPrefix.end());
        static const std::string DocNewLineRepl("\n" + common::doxygenPrefixStr() + "    ");
        ba::replace_all(desc, "\n", DocNewLineRepl);
    }
    return desc;
}

std::string Frame::getIncludes() const
{
    common::StringsList includes;
//    for (auto& f : m_layers) {
//        f->updateIncludes(includes);
//    }

//    if (!m_layers.empty()) {
//        common::mergeInclude("<tuple>", includes);
//    }

    common::mergeInclude(m_generator.mainNamespace() + '/' + common::defaultOptionsStr() + common::headerSuffix(), includes);
    return common::includesToStatements(includes);
}

std::string Frame::getLayersDef() const
{
    std::string result;
    auto scope =
        "TOpt::" +
        m_generator.scopeForFrame(externalRef(), false, true) +
        common::layersSuffixStr() +
        "::";

    for (auto iter = m_layers.rbegin(); iter != m_layers.rend(); ++iter) {
        auto& f = *iter;
        result += f->getClassDefinition(scope);
        if (&f != &m_layers.back()) {
            result += '\n';
        }
    }
    return result;

}

//std::string Frame::getLayersAccessDoc() const
//{
//    if (m_layers.empty()) {
//        return common::emptyString();
//    }

//    std::string result;
//    for (auto& f : m_layers) {
//        if (!result.empty()) {
//            result += '\n';
//        }
//        result += common::doxygenPrefixStr();
//        result += common::indentStr();
//        result += "@li @b transportField_";
//        result += common::nameToAccessCopy(f->name());
//        result += "() for @ref ";
//        result += common::nameToClassCopy(m_dslObj.name());
//        result += "Layers::";
//        result += common::nameToClassCopy(f->name());
//        result += " field.";
//    }

//    return result;
//}

//std::string Frame::getLayersDef() const
//{
//    std::string result;

//    for (auto& f : m_layers) {
//        result += f->getClassDefinition(common::emptyString());
//        if (&f != &m_layers.back()) {
//            result += '\n';
//        }
//    }
//    return result;
//}

//std::string Frame::getLayersOpts() const
//{
//    std::string result =
//        "comms::option::ExtraTransportLayers<" +
//        common::nameToClassCopy(m_dslObj.name()) +
//        common::LayersSuffixStr() +
//        "::All>";

//    auto iter =
//        std::find_if(
//            m_layers.begin(), m_layers.end(),
//            [](auto& f)
//            {
//                return f->semanticType() == commsdsl::Field::SemanticType::Version;
//            });

//    if (iter != m_layers.end()) {
//        result += ",\n";
//        result += "comms::option::VersionInExtraTransportLayers<";
//        result += common::numToString(static_cast<std::size_t>(std::distance(m_layers.begin(), iter)));
//        result += ">";
//    }
//    return result;
//}

}
