#include "IntField.h"

namespace commsdsl2comms
{

const Field::IncludesList& IntField::extraIncludesImpl() const
{
    static const IncludesList List = {
        "comms/field/IntValue.h"
    };
    return List;
}

} // namespace commsdsl2comms
