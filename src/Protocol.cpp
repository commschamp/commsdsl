#include "bbmp/Protocol.h"

#include "ProtocolImpl.h"

namespace bbmp
{

Protocol::Protocol()
  : m_pImpl(new ProtocolImpl)
{

}

Protocol::~Protocol() = default;

bool Protocol::parse(const std::string& input)
{
    return m_pImpl->parse(input);
}

} // namespace bbmp
