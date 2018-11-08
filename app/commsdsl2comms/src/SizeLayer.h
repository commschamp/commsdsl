#pragma once

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class SizeLayer : public Layer
{
    using Base = Layer;
public:
    SizeLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override final;

private:
    commsdsl::SizeLayer sizeLayerDslObj() const
    {
        return commsdsl::SizeLayer(dslObj());
    }
};

inline
LayerPtr createSizeLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<SizeLayer>(generator, layer);
}

} // namespace commsdsl2comms
