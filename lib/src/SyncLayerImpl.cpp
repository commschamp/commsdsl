#include "SyncLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

SyncLayerImpl::SyncLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind SyncLayerImpl::kindImpl() const
{
    return Kind::Sync;
}

} // namespace commsdsl
