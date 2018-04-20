#include "bbmp/Namespace.h"

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
    return m_pImpl->name();
}

Namespace::FieldsList Namespace::fields() const
{
    return m_pImpl->fieldsList();
}


} // namespace bbmp
