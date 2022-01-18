#include "CommsIntField.h"

#include "CommsGenerator.h"

namespace commsdsl2new
{

CommsIntField::CommsIntField(
    CommsGenerator& generator, 
    commsdsl::parse::Field dslObj, 
    commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent)
{
}

bool CommsIntField::writeImpl()
{
    if (!isWritable(getParent())) {
        return true;
    }

    // TODO: write code
    return true;
}

} // namespace commsdsl2new
