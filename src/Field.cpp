#include "Field.h"

#include <functional>
#include <type_traits>
#include <cassert>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "IntField.h"
#include "common.h"

namespace ba = boost::algorithm;

namespace commsdsl2comms
{

namespace
{

Field::IncludesList prepareCommonIncludes(const Generator& generator)
{
    Field::IncludesList list = {
        "comms/Field.h",
        "comms/options.h",
        generator.mainNamespace() + '/' + common::fieldBaseStr() + common::headerSuffix(),
    };

    return list;
}

} // namespace

void Field::updateIncludes(Field::IncludesList& includes) const
{
    static const IncludesList CommonIncludes = prepareCommonIncludes(m_generator);
    common::mergeIncludes(CommonIncludes, includes);
    common::mergeIncludes(extraIncludesImpl(), includes);
}

bool Field::doesExist() const
{
    return
        m_generator.doesElementExist(
            m_dslObj.sinceVersion(),
            m_dslObj.deprecatedSince(),
                m_dslObj.sinceVersion());
}

bool Field::prepare()
{
    m_externalRef = m_dslObj.externalRef();
    return prepareImpl();
}

std::string Field::getClassDefinition(const std::string& scope) const
{
    std::string prefix = "/// @brief Definition of <b>\"";
    prefix += getDisplayName();
    prefix += "\"<\\b> field.\n";

    auto& desc = m_dslObj.description();
    if (!desc.empty()) {
        prefix += "/// @details\n";
        auto multiDesc = common::makeMultiline(desc);
        common::insertIndent(multiDesc);
        auto& doxygenPrefix = common::doxygenPrefixStr();
        multiDesc.insert(multiDesc.begin(), doxygenPrefix.begin(), doxygenPrefix.end());
        ba::replace_all(multiDesc, "\n", "\n" + doxygenPrefix);
        prefix += multiDesc;
        prefix += '\n';
    }

    if (!m_externalRef.empty()) {
        assert(!"NYI: add exter template parameters");
    }

    prefix += getClassDefinitionImpl(scope);
    return prefix;
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
        /* Ref */ [](Generator&, commsdsl::Field) { return Ptr(); },
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

std::string Field::getDefaultOptions() const
{
    return "using = " + common::nameToClassCopy(name()) + " = comms::option::EmptyOption;\n";
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


const std::string& Field::getDisplayName() const
{
    auto* displayName = &m_dslObj.displayName();
    if (displayName->empty()) {
        displayName = &m_dslObj.name();
    }
    return *displayName;
}

} // namespace commsdsl2comms
