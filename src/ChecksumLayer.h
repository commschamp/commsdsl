#pragma once

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class ChecksumLayer : public Layer
{
    using Base = Layer;
public:
    ChecksumLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

    const std::string& getFromLayerName() const
    {
        return checksumLayerDslObj().fromLayer();
    }
protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override;
    virtual bool rearangeImpl(LayersList& layers, bool& success) override;

private:
    commsdsl::ChecksumLayer checksumLayerDslObj() const
    {
        return commsdsl::ChecksumLayer(dslObj());
    }

    std::string getAlg() const;

    bool m_rearanged = false;
};

inline
LayerPtr createChecksumLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<ChecksumLayer>(generator, layer);
}

} // namespace commsdsl2comms
