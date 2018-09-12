#pragma once

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class CustomLayer : public Layer
{
    using Base = Layer;
public:
    CustomLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

    bool isIdReplacement() const
    {
        return customLayerDslObj().isIdReplacement();
    }

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override;
    virtual bool isCustomizableImpl() const override;

private:
    commsdsl::CustomLayer customLayerDslObj() const
    {
        return commsdsl::CustomLayer(dslObj());
    }
};

inline
LayerPtr createCustomLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<CustomLayer>(generator, layer);
}

} // namespace commsdsl2comms
