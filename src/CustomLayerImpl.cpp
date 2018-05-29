#include "CustomLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

CustomLayerImpl::CustomLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind CustomLayerImpl::kindImpl() const
{
    return Kind::Custom;
}

} // namespace commsdsl
