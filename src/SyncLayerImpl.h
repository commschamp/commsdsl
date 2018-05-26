#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class SyncLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    SyncLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual Kind kindImpl() const override;
};

} // namespace commsdsl
