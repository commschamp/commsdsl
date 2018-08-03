#include "IdLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

IdLayerImpl::IdLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind IdLayerImpl::kindImpl() const
{
    return Kind::Id;
}

bool IdLayerImpl::verifyImpl(const LayerImpl::LayersList& layers)
{
    return verifySingleLayer(layers, common::idStr()) &&
           verifyBeforePayload(layers);
}

} // namespace commsdsl
