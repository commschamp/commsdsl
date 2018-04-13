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

std::size_t Field::minLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->minLength();
}

std::size_t Field::maxLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->maxLength();
}

std::size_t Field::bitLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->bitLength();
}

unsigned Field::sinceVersion() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getSinceVersion();
}

unsigned Field::deprecatedSince() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getDeprecated();
}

bool Field::isDeprecatedRemoved() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isDeprecatedRemoved();
}


} // namespace bbmp
