#include "commsdsl/Namespace.h"

#include <cassert>

#include "NamespaceImpl.h"

namespace commsdsl
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

const std::string& Namespace::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Namespace::NamespacesList Namespace::namespaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->namespacesList();
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

Namespace::InterfacesList Namespace::interfaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->interfacesList();
}

Namespace::FramesList Namespace::frames() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->framesList();
}

const Namespace::AttributesMap& Namespace::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Namespace::ElementsList& Namespace::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}


} // namespace commsdsl
