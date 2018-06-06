#include "Field.h"

#include <functional>
#include <type_traits>
#include <cassert>
#include <algorithm>

#include "Generator.h"
#include "IntField.h"
#include "common.h"

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

bool Field::prepareImpl()
{
    return true;
}

const Field::IncludesList& Field::extraIncludesImpl() const
{
    static const IncludesList List;
    return List;
}

} // namespace commsdsl2comms
