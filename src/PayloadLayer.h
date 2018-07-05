#pragma once

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class PayloadLayer : public Layer
{
    using Base = Layer;
public:
    PayloadLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope) const override;

private:
    commsdsl::PayloadLayer payloadLayerDslObj() const
    {
        return commsdsl::PayloadLayer(dslObj());
    }

    std::string getExtraOpt(const std::string& scope) const;
};

inline
LayerPtr createPayloadLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<PayloadLayer>(generator, layer);
}

} // namespace commsdsl2comms
