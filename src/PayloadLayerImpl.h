#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class PayloadLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    PayloadLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    PayloadLayerImpl(const PayloadLayerImpl&) = default;

protected:
    virtual Kind kindImpl() const override;
    virtual bool verifyImpl(const LayersList& layers) override;

};

} // namespace commsdsl
