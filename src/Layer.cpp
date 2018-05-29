#include "commsdsl/Layer.h"
#include <cassert>

#include "LayerImpl.h"
#include "PayloadLayerImpl.h"
#include "IdLayerImpl.h"
#include "SizeLayerImpl.h"
#include "SyncLayerImpl.h"
#include "ChecksumLayerImpl.h"
#include "ValueLayerImpl.h"

namespace commsdsl
{

namespace
{

const ChecksumLayerImpl* asChecksum(const LayerImpl* layer)
{
    assert(layer != nullptr);
    return static_cast<const ChecksumLayerImpl*>(layer);
}

const ValueLayerImpl* asValue(const LayerImpl* layer)
{
    assert(layer != nullptr);
    return static_cast<const ValueLayerImpl*>(layer);
}


} // namespace

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

SizeLayer::SizeLayer(const SizeLayerImpl* impl)
  : Base(impl)
{
}

SizeLayer::SizeLayer(Layer layer)
  : Base(layer)
{
    assert(kind() == Kind::Size);
}

SyncLayer::SyncLayer(const SyncLayerImpl* impl)
  : Base(impl)
{
}

SyncLayer::SyncLayer(Layer layer)
  : Base(layer)
{
    assert(kind() == Kind::Sync);
}

ChecksumLayer::ChecksumLayer(const ChecksumLayerImpl* impl)
  : Base(impl)
{
}

ChecksumLayer::ChecksumLayer(Layer layer)
  : Base(layer)
{
    assert(kind() == Kind::Checksum);
}

ChecksumLayer::Alg ChecksumLayer::alg() const
{
    assert(valid());
    return asChecksum(m_pImpl)->alg();
}

const std::string& ChecksumLayer::customAlgName() const
{
    assert(valid());
    return asChecksum(m_pImpl)->algName();
}

const std::string& ChecksumLayer::fromLayer() const
{
    assert(valid());
    return asChecksum(m_pImpl)->from();
}

const std::string& ChecksumLayer::untilLayer() const
{
    assert(valid());
    return asChecksum(m_pImpl)->until();
}

bool ChecksumLayer::verifyBeforeRead() const
{
    assert(valid());
    return asChecksum(m_pImpl)->verifyBeforeRead();
}

ValueLayer::ValueLayer(const ValueLayerImpl* impl)
  : Base(impl)
{
}

ValueLayer::ValueLayer(Layer layer)
  : Base(layer)
{
    assert(kind() == Kind::Value);
}

ValueLayer::Interfaces ValueLayer::interfaces() const
{
    assert(valid());
    return asValue(m_pImpl)->interfacesList();
}

const std::string& ValueLayer::fieldName() const
{
    assert(valid());
    return asValue(m_pImpl)->fieldName();
}

std::size_t ValueLayer::fieldIdx() const
{
    assert(valid());
    return asValue(m_pImpl)->fieldIdx();
}

bool ValueLayer::pseudo() const
{
    assert(valid());
    return asValue(m_pImpl)->pseudo();
}

} // namespace commsdsl
