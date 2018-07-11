#pragma once

#include <string>

#include "CommsdslApi.h"
#include "Schema.h"
#include "Field.h"
#include "Interface.h"

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
        Custom,
        Sync,
        Size,
        Id,
        Value,
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

class CustomLayerImpl;
class COMMSDSL_API CustomLayer : public Layer
{
    using Base = Layer;
public:
    explicit CustomLayer(const CustomLayerImpl* impl);
    explicit CustomLayer(Layer layer);
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

class SizeLayerImpl;
class COMMSDSL_API SizeLayer : public Layer
{
    using Base = Layer;
public:
    explicit SizeLayer(const SizeLayerImpl* impl);
    explicit SizeLayer(Layer layer);
};

class SyncLayerImpl;
class COMMSDSL_API SyncLayer : public Layer
{
    using Base = Layer;
public:
    explicit SyncLayer(const SyncLayerImpl* impl);
    explicit SyncLayer(Layer layer);
};

class ChecksumLayerImpl;
class COMMSDSL_API ChecksumLayer : public Layer
{
    using Base = Layer;
public:
    enum class Alg
    {
        Custom,
        Sum,
        Crc_CCITT,
        Crc_16,
        Crc_32,
        NumOfValues
    };

    explicit ChecksumLayer(const ChecksumLayerImpl* impl);
    explicit ChecksumLayer(Layer layer);

    Alg alg() const;
    const std::string& customAlgName() const;
    const std::string& fromLayer() const;
    const std::string& untilLayer() const;
    bool verifyBeforeRead() const;
};

class ValueLayerImpl;
class COMMSDSL_API ValueLayer : public Layer
{
    using Base = Layer;
public:
    using Interfaces = std::vector<Interface>;

    explicit ValueLayer(const ValueLayerImpl* impl);
    explicit ValueLayer(Layer layer);

    Interfaces interfaces() const;
    const std::string& fieldName() const;
    std::size_t fieldIdx() const;
    bool pseudo() const;
};


} // namespace commsdsl
