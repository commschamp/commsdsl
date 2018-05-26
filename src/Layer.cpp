#include "commsdsl/Layer.h"
#include <cassert>

#include "LayerImpl.h"
#include "PayloadLayerImpl.h"
#include "IdLayerImpl.h"

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

bool Layer::hasField() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->hasField();
}

Field Layer::field() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->field();
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

PayloadLayer::PayloadLayer(const PayloadLayerImpl* impl)
  : Base(impl)
{
}

PayloadLayer::PayloadLayer(Layer layer)
  : Base(layer)
{
    assert(kind() == Kind::Payload);
}

IdLayer::IdLayer(const IdLayerImpl* impl)
  : Base(impl)
{
}

IdLayer::IdLayer(Layer layer)
  : Base(layer)
{
    assert(kind() == Kind::Id);
}


} // namespace commsdsl
