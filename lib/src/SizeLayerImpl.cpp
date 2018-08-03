#include "SizeLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

SizeLayerImpl::SizeLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind SizeLayerImpl::kindImpl() const
{
    return Kind::Size;
}

bool SizeLayerImpl::verifyImpl(const LayerImpl::LayersList& layers)
{
    return verifySingleLayer(layers, common::sizeStr()) &&
           verifyBeforePayload(layers);
}

} // namespace commsdsl
