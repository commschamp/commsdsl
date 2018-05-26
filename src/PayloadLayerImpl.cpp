#include "PayloadLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

PayloadLayerImpl::PayloadLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind PayloadLayerImpl::kindImpl() const
{
    return Kind::Payload;
}

bool PayloadLayerImpl::verifyImpl(const LayerImpl::LayersList& layers)
{
    return verifySingleLayer(layers, common::payloadStr());
}

bool PayloadLayerImpl::mustHaveFieldImpl() const
{
    return false;
}

} // namespace commsdsl
