#include "bbmp/Field.h"
#include <cassert>

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
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Field::displayName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->displayName();
}

const std::string& Field::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Field::Kind Field::kind() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->kind();
}

std::size_t Field::length() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->length();
}

std::size_t Field::bitLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->bitLength();
}


} // namespace bbmp
