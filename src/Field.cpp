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

const std::string& Field::displayName() const
{
    return m_pImpl->displayName();
}

const std::string& Field::description() const
{
    return m_pImpl->description();
}

Field::Kind Field::kind() const
{
    return m_pImpl->kind();
}


} // namespace bbmp
