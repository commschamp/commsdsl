#pragma once

#include <string>

#include "CommsdslApi.h"
#include "Schema.h"
#include "Field.h"

namespace commsdsl
{

class LayerImpl;
class COMMSDSL_API Layer
{
public:

    using AttributesMap = Schema::AttributesMap;
    using ElementsList = Schema::ElementsList;

    enum class Kind
    {
        Sync,
        Size,
        Id,
        TransportValue,
        Payload,
        Checksum,
        NumOfValues
    };

    explicit Layer(const LayerImpl* impl);
    Layer(const Layer& other);
    ~Layer();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    Kind kind() const;
    bool hasField() const;
    Field field() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const LayerImpl* m_pImpl;
};

class PayloadLayerImpl;
class COMMSDSL_API PayloadLayer : public Layer
{
    using Base = Layer;
public:
    explicit PayloadLayer(const PayloadLayerImpl* impl);
    explicit PayloadLayer(Layer layer);
};

class IdLayerImpl;
class COMMSDSL_API IdLayer : public Layer
{
    using Base = Layer;
public:
    explicit IdLayer(const IdLayerImpl* impl);
    explicit IdLayer(Layer layer);
};

} // namespace commsdsl
