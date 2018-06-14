#include "RefField.h"

#include <type_traits>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

const std::string AliasTemplate(
    "using #^#CLASS_NAME#$# =\n"
    "    #^#REF_FIELD#$#<\n"
    "       #^#OPTS#$#\n"
    "   >;\n"
);

const std::string StructTemplate(
    "struct #^#CLASS_NAME#$# : public\n"
    "    #^#REF_FIELD#$#<\n"
    "       #^#OPTS#$#\n"
    "   >\n"
    "{\n"
    "    #^#NAME_FUNC#$#\n"
    "};\n"
);

} // namespace

bool RefField::prepareImpl()
{
    m_includes.push_back(generator().headerfileForField(refFieldDslObj().field().externalRef(), false));
    return true;
}

const Field::IncludesList& RefField::extraIncludesImpl() const
{
    return m_includes;
}

std::string RefField::getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const
{
    auto refObj = refFieldDslObj().field();
    auto fieldPtr = generator().findField(refObj.externalRef());
    if (fieldPtr == nullptr) {
        assert(!"Unexpected");
        return common::emptyString();
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("CLASS_NAME", common::nameToClassCopy(dslObj().name()) + suffix));
    replacements.insert(std::make_pair("NAME_FUNC", getNameFunc()));
    replacements.insert(
        std::make_pair(
            "REF_FIELD",
            generator().scopeForField(
                refFieldDslObj().field().externalRef(), true, true)));
    replacements.insert(std::make_pair("OPTS", getOpts(scope)));

    auto* templ = &AliasTemplate;
    if (getDisplayName() != fieldPtr->getDisplayName()) {
        templ = &StructTemplate;
    }
    return common::processTemplate(*templ, replacements);
}

std::string RefField::getOpts(const std::string& scope) const
{
    StringsList options;
    options.push_back("TOpt");
    updateExtraOptions(scope, options);
    return common::listToString(options, ",\n", common::emptyString());
}

} // namespace commsdsl2comms
