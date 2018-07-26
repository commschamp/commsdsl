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
#include "ValueLayer.h"
#include "ChecksumLayer.h"
#include "CustomLayer.h"

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

void Layer::updatePluginIncludes(Layer::IncludesList& includes) const
{
    if (m_field) {
        m_field->updatePluginIncludes(includes);
        return;
    }

    auto dslField = m_dslObj.field();
    if (!dslField.valid()) {
        return;
    }

    auto extRef = dslField.externalRef();
    assert (!extRef.empty());
    auto* fieldPtr = m_generator.findField(extRef, false);
    assert(fieldPtr != nullptr);
    fieldPtr->updatePluginIncludes(includes);
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
        /* Custom */ [](Generator& g, commsdsl::Layer l) { return createCustomLayer(g, l); },
        /* Sync */ [](Generator& g, commsdsl::Layer l) { return createSyncLayer(g, l); },
        /* Size */ [](Generator& g, commsdsl::Layer l) { return createSizeLayer(g, l); },
        /* Id */ [](Generator& g, commsdsl::Layer l) { return createIdLayer(g, l); },
        /* Value */ [](Generator& g, commsdsl::Layer l) { return createValueLayer(g, l); },
        /* Payload */ [](Generator& g, commsdsl::Layer l) { return createPayloadLayer(g, l); },
        /* Checksum */ [](Generator& g, commsdsl::Layer l) { return createChecksumLayer(g, l); }
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

std::string Layer::getFieldScopeForPlugin(const std::string& scope) const
{
    if (m_field) {
        return
            scope + common::nameToClassCopy(name()) + common::membersSuffixStr() +
            "::" + common::nameToClassCopy(m_field->name());
    }

    if (kind() != commsdsl::Layer::Kind::Payload) {
        auto dslField = m_dslObj.field();
        assert(dslField.valid());
        auto extRef = dslField.externalRef();
        assert(!extRef.empty());
        return m_generator.scopeForField(extRef, true, true);
    }

    return scope + common::nameToClassCopy(name()) + "::Field";
}

std::string Layer::getFieldAccNameForPlugin() const
{
    auto dslField = m_dslObj.field();
    if (dslField.valid()) {
        return common::nameToAccessCopy(dslField.name());
    }

    return common::nameToAccessCopy(name());
}

std::string Layer::getPluginCreatePropsFunc(const std::string& scope) const
{
    std::string func;
    do {
        if (m_field) {
            auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
            func = m_field->getPluginCreatePropsFunc(fullScope, false, false);
            break;
        }

        auto dslField = m_dslObj.field();
        if (dslField.valid()) {
            auto extRef = dslField.externalRef();
            assert(!extRef.empty());
            auto extScope = m_generator.scopeForFieldInPlugin(extRef);

            static const std::string Templ =
                "QVariantMap createProps_#^#NAME#$#()\n"
                "{\n"
                "    return #^#EXT_SCOPE#$#createProps_#^#EXT_NAME#$#(\"#^#DISP_NAME#$#\");\n"
                "}\n";


            auto* dispName = &dslField.displayName();
            if (dispName->empty()) {
                dispName = &dslField.name();
            }

            common::ReplacementMap replacements;
            replacements.insert(std::make_pair("NAME", getFieldAccNameForPlugin()));
            replacements.insert(std::make_pair("EXT_SCOPE", std::move(extScope)));
            replacements.insert(std::make_pair("EXT_NAME", common::nameToAccessCopy(dslField.name())));
            replacements.insert(std::make_pair("DISP_NAME", *dispName));
            func = common::processTemplate(Templ, replacements);
            break;
        }

        static const std::string Templ =
            "QVariantMap createProps_#^#NAME#$#()\n"
            "{\n"
            "    return cc::property::field::ArrayList().name(#^#DISP_NAME#$#).asMap();\n"
            "}\n";

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("NAME", getFieldAccNameForPlugin()));
        replacements.insert(std::make_pair("DISP_NAME", name()));
        func = common::processTemplate(Templ, replacements);
    } while (false);

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#Layer\n"
        "{\n"
        "    #^#FUNC#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("FUNC", std::move(func)));
    return common::processTemplate(Templ, replacements);
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

std::string Layer::getExtraOpt(const std::string& scope) const
{
    return "typename " + scope + common::nameToClassCopy(name());
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

void Layer::updateIncludesImpl(IncludesList& includes) const
{
    static_cast<void>(includes);
}

std::string Layer::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    static_cast<void>(scope);
    return common::emptyString();
}

bool Layer::rearangeImpl(LayersList& layers, bool& success)
{
    static_cast<void>(layers);
    success = true;
    return false;
}

} // namespace commsdsl2comms
