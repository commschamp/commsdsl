#include "BundleField.h"

#include <type_traits>
#include <numeric>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"
#include "OptionalField.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string MembersDefTemplate =
    "/// @brief Scope for all the member fields of @ref #^#CLASS_NAME#$# bitfield.\n"
    "#^#EXTRA_PREFIX#$#\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#MEMBERS_DEFS#$#\n"
    "    /// @brief All members bundled in @b std::tuple.\n"
    "    using All =\n"
    "        std::tuple<\n"
    "           #^#MEMBERS#$#\n"
    "        >;\n"
    "};\n";

const std::string MembersOptionsTemplate =
    "/// @brief Extra options for all the member fields of @ref #^#SCOPE#$##^#CLASS_NAME#$# bitfield.\n"
    "struct #^#CLASS_NAME#$#Members\n"
    "{\n"
    "    #^#OPTIONS#$#\n"
    "};\n";

const std::string ClassTemplate(
    "#^#MEMBERS_STRUCT_DEF#$#\n"
    "#^#PREFIX#$#"
    "class #^#CLASS_NAME#$# : public\n"
    "    comms::field::Bundle<\n"
    "        #^#PROT_NAMESPACE#$#::FieldBase<>,\n"
    "        typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
    "        #^#FIELD_OPTS#$#\n"
    "    >\n"
    "{\n"
    "    using Base = \n"
    "        comms::field::Bundle<\n"
    "            #^#PROT_NAMESPACE#$#::FieldBase<>,\n"
    "            typename #^#CLASS_NAME#$#Members#^#MEMBERS_OPT#$#::All#^#COMMA#$#\n"
    "            #^#FIELD_OPTS#$#\n"
    "        >;\n"
    "public:\n"
    "    #^#ACCESS#$#\n"
    "    #^#NAME#$#\n"
    "    #^#READ#$#\n"
    "    #^#WRITE#$#\n"
    "    #^#LENGTH#$#\n"
    "    #^#VALID#$#\n"
    "    #^#REFRESH#$#\n"
    "#^#PRIVATE#$#\n"
    "};\n"
);

} // namespace

bool BundleField::prepareImpl()
{
    auto obj = bundleFieldDslObj();
    auto members = obj.members();
    m_members.reserve(members.size());
    for (auto& m : members) {
        auto ptr = create(generator(), m);
        if (!ptr) {
            assert(!"should not happen");
            return false;
        }

        if (!ptr->prepare(obj.sinceVersion())) {
            return false;
        }

        m_members.push_back(std::move(ptr));
    }
    return true;
}

void BundleField::updateIncludesImpl(IncludesList& includes) const
{
    static const IncludesList List = {
        "comms/field/Bundle.h",
        "<tuple>"
    };

    common::mergeIncludes(List, includes);

    for (auto& m : m_members) {
        m->updateIncludes(includes);
    }
}

void BundleField::updatePluginIncludesImpl(Field::IncludesList& includes) const
{
    for (auto& m : m_members) {
        m->updatePluginIncludes(includes);
    }
}

std::size_t BundleField::minLengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), std::size_t(0),
            [](std::size_t soFar, auto& m)
            {
                return soFar + m->minLength();
            });
}

std::string BundleField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PREFIX", getClassPrefix(suffix)));
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("PROT_NAMESPACE", generator().mainNamespace()));
    replacements.insert(std::make_pair("FIELD_OPTS", getFieldOpts(scope)));
    replacements.insert(std::make_pair("NAME", getNameFunc()));
    replacements.insert(std::make_pair("READ", getRead()));
    replacements.insert(std::make_pair("WRITE", getCustomWrite()));
    replacements.insert(std::make_pair("LENGTH", getCustomLength()));
    replacements.insert(std::make_pair("VALID", getCustomValid()));
    replacements.insert(std::make_pair("REFRESH", getRefresh()));
    replacements.insert(std::make_pair("MEMBERS_STRUCT_DEF", getMembersDef(scope, suffix)));
    replacements.insert(std::make_pair("ACCESS", getAccess(suffix)));
    replacements.insert(std::make_pair("PRIVATE", getPrivate()));
    if (!replacements["FIELD_OPTS"].empty()) {
        replacements["COMMA"] = ',';
    }

    if (!externalRef().empty()) {
        replacements.insert(std::make_pair("MEMBERS_OPT", "<TOpt>"));
    }

    return common::processTemplate(ClassTemplate, replacements);
}

std::string BundleField::getExtraDefaultOptionsImpl(const std::string& scope) const
{
    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    StringsList options;
    options.reserve(m_members.size());
    for (auto& m : m_members) {
        options.push_back(m->getDefaultOptions(memberScope));
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("SCOPE", scope));
    replacements.insert(std::make_pair("OPTIONS", common::listToString(options, "\n", common::emptyString())));
    return common::processTemplate(MembersOptionsTemplate, replacements);
}

std::string BundleField::getPluginAnonNamespaceImpl(const std::string& scope) const
{
    auto fullScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    common::StringsList props;
    for (auto& f : m_members) {
        props.push_back(f->getPluginCreatePropsFunc(fullScope));
    }

    static const std::string Templ =
        "struct #^#CLASS_NAME#$#Members\n"
        "{\n"
        "    #^#PROPS#$#\n"
        "};\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name())));
    replacements.insert(std::make_pair("PROPS", common::listToString(props, "\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string BundleField::getPluginPropertiesImpl() const
{
    common::StringsList props;
    props.reserve(m_members.size());
    auto prefix =
        common::nameToClassCopy(name()) + common::membersSuffixStr() +
        "::createProps_";
    for (auto& f : m_members) {
        props.push_back(".add(" + prefix + common::nameToAccessCopy(f->name()) + "())");
    }

    return common::listToString(props, "\n", common::emptyString());
}

std::string BundleField::getFieldOpts(const std::string& scope) const
{
    StringsList options;

    updateExtraOptions(scope, options);

    bool hasConditions =
        std::any_of(
            m_members.begin(), m_members.end(),
            [](auto& m) {
                assert(m);
                if (m->kind() != commsdsl::Field::Kind::Optional) {
                    return false;
                }

                auto* optField = static_cast<const OptionalField*>(m.get());
                auto cond = optField->cond();
                return cond.valid();
            });

    if (hasConditions) {
        common::addToList("comms::option::HasCustomRead", options);
        common::addToList("comms::option::HasCustomRefresh", options);
    }

    return common::listToString(options, ",\n", common::emptyString());
}

std::string BundleField::getMembersDef(const std::string& scope, const std::string& suffix) const
{
    std::string memberScope = scope + common::nameToClassCopy(name()) + common::membersSuffixStr() + "::";
    StringsList membersDefs;
    StringsList membersNames;

    membersDefs.reserve(m_members.size());
    membersNames.reserve(m_members.size());
    for (auto& m : m_members) {
        membersDefs.push_back(m->getClassDefinition(memberScope));
        membersNames.push_back(common::nameToClassCopy(m->name()));
    }

    std::string prefix;
    if (!externalRef().empty()) {
        prefix += "/// @tparam TOpt Protocol options.\n";
        prefix += "template <typename TOpt = " + generator().mainNamespace() + "::" + common::defaultOptionsStr() + ">";
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(name() + suffix)));
    replacements.insert(std::make_pair("EXTRA_PREFIX", std::move(prefix)));
    replacements.insert(std::make_pair("MEMBERS_DEFS", common::listToString(membersDefs, "\n", common::emptyString())));
    replacements.insert(std::make_pair("MEMBERS", common::listToString(membersNames, ",\n", common::emptyString())));
    return common::processTemplate(MembersDefTemplate, replacements);

}

std::string BundleField::getAccess(const std::string& suffix) const
{
    static const std::string Templ =
        "/// @brief Allow access to internal fields.\n"
        "/// @details See definition of @b COMMS_FIELD_MEMBERS_ACCESS macro\n"
        "///     related to @b comms::field::Bundle class from COMMS library\n"
        "///     for details.\n"
        "///\n"
        "///      The generated access functions are:\n"
        "#^#ACCESS_DOC#$#\n"
        "COMMS_FIELD_MEMBERS_ACCESS(\n"
        "    #^#NAMES#$#\n"
        ");\n";

    StringsList accessDocList;
    StringsList namesList;
    accessDocList.reserve(m_members.size());
    namesList.reserve(m_members.size());

    for (auto& m : m_members) {
        namesList.push_back(common::nameToAccessCopy(m->name()));
        std::string accessStr =
            "///     @li @b field_" + namesList.back() +
            "() - for @ref " +
            common::nameToClassCopy(name()) + suffix +
            common::membersSuffixStr() + "::" +
            namesList.back() + " member field.";
        accessDocList.push_back(std::move(accessStr));
        
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("ACCESS_DOC", common::listToString(accessDocList, "\n", common::emptyString())));
    replacements.insert(std::make_pair("NAMES", common::listToString(namesList, ",\n", common::emptyString())));
    return common::processTemplate(Templ, replacements);
}

std::string BundleField::getRead() const
{
    auto customRead = getCustomRead();
    if (!customRead.empty()) {
        return customRead;
    }

    return getReadForFields(m_members, false);
}

std::string BundleField::getRefresh() const
{
    auto customRefresh = getCustomRefresh();
    if (!customRefresh.empty()) {
        return customRefresh;
    }

    return getPublicRefreshForFields(m_members, false);
}

std::string BundleField::getPrivate() const
{
    auto str = getPrivateRefreshForFields(m_members);
    if (str.empty()) {
        return common::emptyString();
    }

    common::insertIndent(str);
    static const std::string Prefix("private:\n");
    return Prefix + str; 
}

} // namespace commsdsl2comms
