#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class CustomLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    CustomLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual Kind kindImpl() const override;
};

} // namespace commsdsl
