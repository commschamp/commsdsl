#pragma once

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class ValueLayer : public Layer
{
    using Base = Layer;
public:
    ValueLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override;

private:
    commsdsl::ValueLayer valueLayerDslObj() const
    {
        return commsdsl::ValueLayer(dslObj());
    }
};

inline
LayerPtr createValueLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<ValueLayer>(generator, layer);
}

} // namespace commsdsl2comms
