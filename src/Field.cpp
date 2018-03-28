#include "bbmp/Field.h"

#include "FieldImpl.h"

namespace bbmp
{

Field::Field(const FieldImpl* impl)
  : m_pImpl(impl)
{
}

Field::Field(const Field &) = default;

Field::~Field() = default;

const std::string& Field::name() const
{
    return m_pImpl->name();
}


} // namespace bbmp
