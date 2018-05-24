#pragma once

#include <string>

#include "CommsdslApi.h"
#include "Schema.h"

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

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const LayerImpl* m_pImpl;
};

} // namespace commsdsl
