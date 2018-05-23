#include "bbmp/Interface.h"
#include <cassert>

#include "InterfaceImpl.h"

namespace bbmp
{

Interface::Interface(const InterfaceImpl* impl)
  : m_pImpl(impl)
{
}

Interface::Interface(const Interface &) = default;

Interface::~Interface() = default;

bool Interface::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Interface::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Interface::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Interface::FieldsList Interface::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

std::string Interface::externalRef() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef();
}

const Interface::AttributesMap& Interface::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Interface::ElementsList& Interface::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace bbmp
