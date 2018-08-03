#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class IdLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    IdLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual Kind kindImpl() const override;
    virtual bool verifyImpl(const LayersList& layers) override;

};

} // namespace commsdsl
