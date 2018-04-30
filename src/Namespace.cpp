#include "bbmp/Namespace.h"

#include <cassert>

#include "NamespaceImpl.h"

namespace bbmp
{

Namespace::Namespace(const NamespaceImpl* impl)
  : m_pImpl(impl)
{
}

Namespace::Namespace(const Namespace &) = default;

Namespace::~Namespace() = default;

const std::string& Namespace::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

Namespace::FieldsList Namespace::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

Namespace::MessagesList Namespace::messages() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->messagesList();
}


} // namespace bbmp
