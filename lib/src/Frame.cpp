#include "commsdsl/Frame.h"
#include <cassert>

#include "FrameImpl.h"

namespace commsdsl
{

Frame::Frame(const FrameImpl* impl)
  : m_pImpl(impl)
{
}

Frame::Frame(const Frame &) = default;

Frame::~Frame() = default;

bool Frame::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Frame::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Frame::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Frame::LayersList Frame::layers() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->layersList();
}

std::string Frame::externalRef() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef();
}

const Frame::AttributesMap& Frame::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Frame::ElementsList& Frame::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace commsdsl
