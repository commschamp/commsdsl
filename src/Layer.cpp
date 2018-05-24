#include "commsdsl/Layer.h"
#include <cassert>

#include "LayerImpl.h"

namespace commsdsl
{

Layer::Layer(const LayerImpl* impl)
  : m_pImpl(impl)
{
}

Layer::Layer(const Layer &) = default;

Layer::~Layer() = default;

bool Layer::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Layer::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Layer::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Layer::Kind Layer::kind() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->kind();
}

const Layer::AttributesMap& Layer::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Layer::ElementsList& Layer::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace commsdsl
