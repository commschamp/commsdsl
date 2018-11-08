#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class PayloadLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    PayloadLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual Kind kindImpl() const override final;
    virtual bool verifyImpl(const LayersList& layers) override final;
    virtual bool mustHaveFieldImpl() const override final;

};

} // namespace commsdsl
