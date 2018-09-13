#include "commsdsl/Message.h"
#include <cassert>

#include "MessageImpl.h"

namespace commsdsl
{

Message::Message(const MessageImpl* impl)
  : m_pImpl(impl)
{
}

Message::Message(const Message &) = default;

Message::~Message() = default;

bool Message::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Message::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Message::displayName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->displayName();
}

const std::string& Message::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

std::uintmax_t Message::id() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->id();
}

unsigned Message::order() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->order();
}

std::size_t Message::minLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->minLength();
}

std::size_t Message::maxLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->maxLength();
}

unsigned Message::sinceVersion() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getSinceVersion();
}

unsigned Message::deprecatedSince() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getDeprecated();
}

bool Message::isDeprecatedRemoved() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isDeprecatedRemoved();
}

Message::FieldsList Message::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

std::string Message::externalRef() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef();
}

bool Message::isCustomizable() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isCustomizable();
}

Message::Sender Message::sender() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->sender();
}

const Message::AttributesMap& Message::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Message::ElementsList& Message::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

const Message::PlatformsList& Message::platforms() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->platforms();
}


} // namespace commsdsl
