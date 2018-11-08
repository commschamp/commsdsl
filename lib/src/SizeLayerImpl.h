#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class SizeLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    SizeLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual Kind kindImpl() const override final;
    virtual bool verifyImpl(const LayersList& layers) override final;

};

} // namespace commsdsl
