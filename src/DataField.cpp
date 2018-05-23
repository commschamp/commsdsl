#include "commsdsl/DataField.h"

#include <cassert>

#include "DataFieldImpl.h"

namespace commsdsl
{

namespace
{

const DataFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const DataFieldImpl*>(ptr);
}

} // namespace

DataField::DataField(const DataFieldImpl* impl)
  : Base(impl)
{
}

DataField::DataField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Data);
}

const DataField::ValueType& DataField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

unsigned DataField::fixedLength() const
{
    return cast(m_pImpl)->length();
}

bool DataField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasPrefixField();
}

Field DataField::lengthPrefixField() const
{
    return cast(m_pImpl)->prefixField();
}

} // namespace commsdsl
