#include "Field.h"

#include <functional>
#include <type_traits>
#include <cassert>
#include <algorithm>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "IntField.h"
#include "RefField.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string FileTemplate(
    "/// @file\n"
    "/// @brief Contains definition of <b>\"#^#FIELD_NAME#$#\"<\\b> field.\n"
    "\n"
    "#pragma once\n"
    "\n"
    "#^#INCLUDES#$#\n"
    "#^#BEGIN_NAMESPACE#$#\n"
    "#^#CLASS_DEF#$#\n"
    "#^#END_NAMESPACE#$#\n"
);

Field::IncludesList prepareCommonIncludes(const Generator& generator)
{
    Field::IncludesList list = {
        "comms/options.h",
        generator.mainNamespace() + '/' + common::fieldBaseStr() + common::headerSuffix(),
    };

    return list;
}

} // namespace

std::size_t Field::minLength() const
{
    if (isVersionOptional()) {
        return 0U;
    }

    return m_dslObj.minLength();
}

void Field::updateIncludes(Field::IncludesList& includes) const
{
    static const IncludesList CommonIncludes = prepareCommonIncludes(m_generator);
    common::mergeIncludes(CommonIncludes, includes);
    common::mergeIncludes(extraIncludesImpl(), includes);
    if (!m_externalRef.empty()) {
        auto inc =
            m_generator.mainNamespace() + '/' + common::defaultOptionsStr() + common::headerSuffix();
        common::mergeInclude(inc, includes);
    }

    if (isVersionOptional()) {
        common::mergeInclude("comms/field/Optional.h", includes);
    }
}

bool Field::doesExist() const
{
    return
        m_generator.doesElementExist(
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
            m_dslObj.isDeprecatedRemoved());
}

bool Field::prepare(unsigned parentVersion)
{
    m_externalRef = m_dslObj.externalRef();
    m_parentVersion = parentVersion;
    return prepareImpl();
}

std::string Field::getClassDefinition(
    const std::string& scope,
    const std::string& suffix) const
{
    std::string prefix = "/// @brief Definition of <b>\"";
    prefix += getDisplayName();
    prefix += "\"<\\b> field.\n";

    std::string str;
    bool optional = isVersionOptional();
    if (optional) {
        str = "/// @brief Inner field of @ref " + common::nameToClassCopy(name()) + " optional.\n";
    }
    else {
        str = prefix;
    }

    auto& desc = m_dslObj.description();
    if (!desc.empty()) {
        str += "/// @details\n";
        auto multiDesc = common::makeMultiline(desc);
        common::insertIndent(multiDesc);
        auto& doxygenPrefix = common::doxygenPrefixStr();
        multiDesc.insert(multiDesc.begin(), doxygenPrefix.begin(), doxygenPrefix.end());
        ba::replace_all(multiDesc, "\n", "\n" + doxygenPrefix);
        str += multiDesc;
        str += '\n';
    }

    auto addExternalRefPrefix =
        [this, &str]()
        {
            if (!m_externalRef.empty()) {
                str += "/// @tparam TOpt Protocol options.\n";
                str += "/// @tparam TExtraOpts Extra options.\n";
                str += "template <typename TOpt = ";
                str += m_generator.mainNamespace();
                str += "::";
                str += common::defaultOptionsStr();
                str += ", typename... TExtraOpts>\n";
            }
        };


    addExternalRefPrefix();

    std::string classNameSuffix = suffix;
    if (optional) {
        classNameSuffix += common::optFieldSuffixStr();
    }

    str += getClassDefinitionImpl(scope, classNameSuffix);

    if (optional) {
        str += '\n';
        str += prefix;
        addExternalRefPrefix();

        static const std::string Templ =
            "using #^#CLASS_NAME#$# =\n"
            "    comms::field::Optional<\n"
            "        #^#CLASS_NAME#$#Field#^#FIELD_PARAMS#$#,\n"
            "        comms::option::#^#DEFAULT_MODE_OPT#$#,\n"
            "        comms::option::#^#VERSIONS_OPT#$#\n"
            "    >;\n";

        std::string fieldParams;
        if (!m_externalRef.empty()) {
            fieldParams = "<TOpt, TExtraOpts...>";
        }

        std::string defaultModeOpt = "ExistsByDefault";
        if (!doesExist()) {
            defaultModeOpt = "MissingByDefault";
        }

        std::string versionOpt = "ExistsSinceVersion<" + common::numToString(m_dslObj.sinceVersion()) + '>';
        if (m_dslObj.isDeprecatedRemoved()) {
            assert(m_dslObj.deprecatedSince() < commsdsl::Protocol::notYetDeprecated());
            if (m_dslObj.sinceVersion() == 0U) {
                versionOpt = "ExistsUntilVersion<" + common::numToString(m_dslObj.deprecatedSince()) + '>';
            }
            else {
                versionOpt =
                    "ExistsBetweenVersions<" +
                    common::numToString(m_dslObj.sinceVersion()) +
                    ", " +
                    common::numToString(m_dslObj.deprecatedSince()) +
                    '>';
            }
        }

        common::ReplacementMap replacements;
        replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(m_dslObj.name()) + suffix));
        replacements.insert(std::make_pair("FIELD_PARAMS", std::move(fieldParams)));
        replacements.insert(std::make_pair("DEFAULT_MODE_OPT", std::move(defaultModeOpt)));
        replacements.insert(std::make_pair("VERSIONS_OPT", std::move(versionOpt)));
        str += common::processTemplate(Templ, replacements);
    }
    return str;
}

Field::Ptr Field::create(Generator& generator, commsdsl::Field field)
{
    using CreateFunc = std::function<Ptr (Generator& generator, commsdsl::Field)>;
    static const CreateFunc Map[] = {
        /* Int */ [](Generator& g, commsdsl::Field f) { return createIntField(g, f); },
        /* Enum */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* Set */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* Float */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* Bitfield */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* Bundle */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* String */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* Data */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* List */ [](Generator&, commsdsl::Field) { return Ptr(); },
        /* Ref */ [](Generator& g, commsdsl::Field f) { return createRefField(g, f); },
        /* Optional */ [](Generator&, commsdsl::Field) { return Ptr(); },
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == (unsigned)commsdsl::Field::Kind::NumOfValues, "Invalid map");

    auto idx = static_cast<std::size_t>(field.kind());
    if (MapSize <= idx) {
        assert(!"Unexpected field kind");
        return Ptr();
    }

    return Map[idx](generator, field);
}

std::string Field::getDefaultOptions(const std::string& scope) const
{
    return
        "/// @brief Extra options for @ref " +
        scope + common::nameToClassCopy(name()) + " field.\n" +
        "using " + common::nameToClassCopy(name()) +
        " = comms::option::EmptyOption;\n";
}

bool Field::writeProtocolDefinition() const
{
    auto startInfo = m_generator.startFieldProtocolWrite(m_externalRef);
    auto& filePath = startInfo.first;
    if (filePath.empty()) {
        return true;
    }

    assert(!m_externalRef.empty());
    IncludesList includes;
    updateIncludes(includes);
    auto incStr = common::includesToStatements(includes);

    auto namespaces = m_generator.namespacesForField(m_externalRef);

    // TODO: modifile class name

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("INCLUDES", std::move(incStr)));
    replacements.insert(std::make_pair("BEGIN_NAMESPACE", std::move(namespaces.first)));
    replacements.insert(std::make_pair("END_NAMESPACE", std::move(namespaces.second)));
    replacements.insert(std::make_pair("CLASS_DEF", getClassDefinition("TOpt::" + m_generator.scopeForField(m_externalRef))));
    replacements.insert(std::make_pair("FIELD_NAME", getDisplayName()));

    std::string str = common::processTemplate(FileTemplate, replacements);

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

const std::string& Field::getDisplayName() const
{
    auto* displayName = &m_dslObj.displayName();
    if (displayName->empty()) {
        displayName = &m_dslObj.name();
    }
    return *displayName;
}

bool Field::prepareImpl()
{
    return true;
}

const Field::IncludesList& Field::extraIncludesImpl() const
{
    static const IncludesList List;
    return List;
}


std::string Field::getNameFunc() const
{
    return
        "/// @brief Name of the field.\n"
        "static const char* name()\n"
        "{\n"
        "    return \"" + getDisplayName() + "\";\n"
                                             "}\n";
}

void Field::updateExtraOptions(const std::string& scope, common::StringsList& options) const
{
    if (!m_externalRef.empty()) {
        options.push_back("TExtraOpts...");
    }

    if (!scope.empty()) {
        options.push_back("typename " + scope + common::nameToClassCopy(name()));
    }
}

std::string Field::getCustomRead() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomWrite() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomLength() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomValid() const
{
    // TODO: implement
    return common::emptyString();
}

std::string Field::getCustomRefresh() const
{
    // TODO: implement
    return common::emptyString();
}

bool Field::isVersionOptional() const
{
    return
        (m_generator.versionDependentCode()) &&
        (m_parentVersion < m_dslObj.sinceVersion()) &&
        (m_generator.isElementOptional(
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
            m_dslObj.isDeprecatedRemoved()));
}

} // namespace commsdsl2comms
