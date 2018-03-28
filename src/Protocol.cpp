#include "bbmp/Protocol.h"

#include "ProtocolImpl.h"

namespace bbmp
{

Protocol::Protocol()
  : m_pImpl(new ProtocolImpl)
{

}

void Protocol::setErrorReportCallback(Protocol::ErrorReportFunction&& cb)
{
    m_pImpl->setErrorReportCallback(std::move(cb));
}

Protocol::~Protocol() = default;

bool Protocol::parse(const std::string& input)
{
    return m_pImpl->parse(input);
}

bool Protocol::validate()
{
    return m_pImpl->validate();
}

Schema Protocol::schema() const
{
    return m_pImpl->schema();
}

const Protocol::NamespacesList&Protocol::namespaces() const
{
    return m_pImpl->namespacesList();
}

} // namespace bbmp
