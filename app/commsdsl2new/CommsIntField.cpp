#include "CommsIntField.h"

#include "CommsGenerator.h"

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
        "<cstdint>",
        "<type_traits>",
        "<utility>"
    };

    return list;
}

} // namespace commsdsl2new
