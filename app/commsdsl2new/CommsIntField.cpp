#include "CommsIntField.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2new
{

CommsIntField::CommsIntField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    CommsBase(static_cast<Base&>(*this))
{
}

bool CommsIntField::writeImpl() const
{
    return commsWrite();
}

CommsIntField::IncludesList CommsIntField::commsCommonIncludesImpl() const
{
    IncludesList list = {
        "<cstdint>"
    };

    auto& specials = specialsSortedByValue(); 
    if (!specials.empty()) {
        list.insert(list.end(),
            {
                "<type_traits>", 
                "<utility>"
            });
    }

    return list;
}

std::string CommsIntField::commsCommonCodeBodyImpl() const
{
    static const std::string Templ = 
        "/// @brief Re-definition of the value type used by\n"
        "///     #^#SCOPE#$# field.\n"
        "using ValueType = #^#VALUE_TYPE#$#;\n\n"
        "#^#SPECIAL_VALUE_NAMES_MAP_DEFS#$#\n"
        "#^#NAME_FUNC#$#\n"
        "#^#HAS_SPECIAL_FUNC#$#\n"
        "#^#SPECIALS#$#\n"
        "#^#SPECIAL_NAMES_MAP#$#\n"   
    ;

    //auto& specials = specialsSortedByValue();

    auto& gen = generator();
    auto dslObj = intDslObj();
    util::ReplacementMap repl = {
        {"SCOPE", comms::scopeFor(*this, gen, true, true)},
        {"VALUE_TYPE", comms::cppIntTypeFor(dslObj.type(), dslObj.maxLength())},
        {"SPECIAL_VALUE_NAMES_MAP_DEFS", commsCommonValueNamesMapCode()},
        {"NAME_FUNC", commsCommonNameFuncCode()},
        {"HAS_SPECIAL_FUNC", commsCommonHasSpecialsFuncCode()},
        {"SPECIALS", commsCommonSpecialsCode()},
        {"SPECIAL_NAMES_MAP", commsCommonSpecialNamesMapCode()},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsIntField::commsCommonHasSpecialsFuncCode() const
{
    static const std::string Templ = 
        "/// @brief Compile time detection of special values presence.\n"
        "static constexpr bool hasSpecials()\n"
        "{\n"
        "    return #^#VALUE#$#;\n"
        "}\n"
    ;

    auto& specials = specialsSortedByValue();    
    util::ReplacementMap repl = {
        {"VALUE", util::boolToString(!specials.empty())}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsIntField::commsCommonValueNamesMapCode() const
{
    auto& specials = specialsSortedByValue();    
    if (specials.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Single special value name info entry.\n"
        "using SpecialNameInfo = #^#INFO_DEF#$#;\n\n"
        "/// @brief Type returned from @ref specialNamesMap() member function.\n"
        "/// @details The @b first value of the pair is pointer to the map array,\n"
        "///     The @b second value of the pair is the size of the array.\n"
        "using SpecialNamesMapInfo = #^#MAP_DEF#$#;\n";

    util::ReplacementMap repl = {
        {"INFO_DEF", "std::pair<ValueType, const char*>"},
        {"MAP_DEF", "std::pair<const SpecialNameInfo*, std::size_t>"}
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsIntField::commsCommonSpecialsCode() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    util::StringsList specialsList;
    for (auto& s : specials) {
        if (!generator().doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ(
            "/// @brief Special value <b>\"#^#SPEC_NAME#$#\"</b>.\n"
            "#^#SPECIAL_DOC#$#\n"
            "static constexpr ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return static_cast<ValueType>(#^#SPEC_VAL#$#);\n"
            "}\n"
        );

        std::string specVal;
        auto obj = intDslObj();
        auto type = obj.type();
        if ((type == commsdsl::parse::IntField::Type::Uint64) ||
            (type == commsdsl::parse::IntField::Type::Uintvar)) {
            specVal = util::numToString(static_cast<std::uintmax_t>(s.second.m_value));
        }
        else {
            specVal = util::numToString(s.second.m_value);
        }

        std::string desc = s.second.m_description;
        if (!desc.empty()) {
            static const std::string Prefix("/// @details ");
            desc.insert(desc.begin(), Prefix.begin(), Prefix.end());
            desc = util::strMakeMultiline(desc);
            desc = util::strReplace(desc, "\n", "\n///     ");
        }

        util::ReplacementMap repl = {
            {"SPEC_NAME", s.first},
            {"SPEC_ACC", comms::className(s.first)},
            {"SPEC_VAL", std::move(specVal)},
            {"SPECIAL_DOC", std::move(desc)},
        };

        specialsList.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(specialsList, "\n", "\n");
}

std::string CommsIntField::commsCommonSpecialNamesMapCode() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        "/// @brief Retrieve map of special value names\n"
        "static SpecialNamesMapInfo specialNamesMap()\n"
        "{\n"
        "    static const SpecialNameInfo Map[] = {\n"
        "        #^#INFOS#$#\n"
        "    };\n"
        "    static const std::size_t MapSize = std::extent<decltype(Map)>::value;\n\n"
        "    return std::make_pair(&Map[0], MapSize);\n"
        "}\n";    

    util::StringsList specialInfos;
    for (auto& s : specials) {
        static const std::string SpecTempl = 
            "std::make_pair(value#^#SPEC_ACC#$#(), \"#^#SPEC_NAME#$#\")";

        util::ReplacementMap specRepl = {
            {"SPEC_ACC", comms::className(s.first)},
            {"SPEC_NAME", s.first}
        };
        specialInfos.push_back(util::processTemplate(SpecTempl, specRepl));
    }

    util::ReplacementMap repl {
        {"INFOS", util::strListToString(specialInfos, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2new
