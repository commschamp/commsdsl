#include "CommsField.h"

#include <cassert>

namespace commsdsl2new
{

bool CommsField::isWritable(const commsdsl::gen::Elem* parent)
{
    if (parent == nullptr) {
        assert(false); // Should not happen
        return false;
    }

    auto type = parent->elemType();
    return (type == commsdsl::gen::Elem::Type::Type_Namespace);
}

} // namespace commsdsl2new
