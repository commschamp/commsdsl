#pragma once

#include <string>

#include "BbmpApi.h"
#include "Schema.h"

namespace bbmp
{

class FieldImpl;
class BBMP_API Field
{
public:

    using AttributesMap = Schema::AttributesMap;
    using ElementsList = Schema::ElementsList;

    enum class Kind
    {
        Int,
        Enum,
        Set,
        Float,
        Bitfield,
        Bundle,
        String,
        Data,
        List,
        Ref,
        Optional,
        NumOfValues
    };

    explicit Field(const FieldImpl* impl);
    Field(const Field& other);
    ~Field();

    bool valid() const;
    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;
    Kind kind() const;
    std::size_t minLength() const;
    std::size_t maxLength() const;
    std::size_t bitLength() const;
    unsigned sinceVersion() const;
    unsigned deprecatedSince() const;
    bool isDeprecatedRemoved() const;
    std::string externalRef() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const FieldImpl* m_pImpl;
};

} // namespace bbmp
