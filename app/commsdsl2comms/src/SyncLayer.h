#pragma once

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class SyncLayer : public Layer
{
    using Base = Layer;
public:
    SyncLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

protected:
    virtual bool prepareImpl() override final;
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override final;

private:
    commsdsl::SyncLayer sizeLayerDslObj() const
    {
        return commsdsl::SyncLayer(dslObj());
    }
};

inline
LayerPtr createSyncLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<SyncLayer>(generator, layer);
}

} // namespace commsdsl2comms
