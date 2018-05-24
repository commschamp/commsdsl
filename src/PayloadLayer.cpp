#include "commsdsl/PayloadLayer.h"

#include <cassert>

#include "PayloadLayerImpl.h"

namespace commsdsl
{

//namespace
//{

//const PayloadLayerImpl* cast(const FieldImpl* ptr)
//{
//    assert(ptr != nullptr);
//    return static_cast<const PayloadLayerImpl*>(ptr);
//}

//} // namespace

PayloadLayer::PayloadLayer(const PayloadLayerImpl* impl)
  : Base(impl)
{
}

PayloadLayer::PayloadLayer(Layer layer)
  : Base(layer)
{
    assert(kind() == Kind::Payload);
}

} // namespace commsdsl
