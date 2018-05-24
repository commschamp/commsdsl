#pragma once

#include "Layer.h"

namespace commsdsl
{

class PayloadLayerImpl;
class COMMSDSL_API PayloadLayer : public Layer
{
    using Base = Layer;
public:
    explicit PayloadLayer(const PayloadLayerImpl* impl);
    explicit PayloadLayer(Layer layer);
};

} // namespace commsdsl
