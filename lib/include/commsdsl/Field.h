#pragma once

#include <string>

#include "CommsdslApi.h"
#include "Schema.h"

namespace commsdsl
{

class FieldImpl;
class COMMSDSL_API Field
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

    enum class SemanticType
    {
        None,
        Version,
        MessageId,
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
    SemanticType semanticType() const;
    std::size_t minLength() const;
    std::size_t maxLength() const;
    std::size_t bitLength() const;
    unsigned sinceVersion() const;
    unsigned deprecatedSince() const;
    bool isDeprecatedRemoved() const;
    std::string externalRef() const;
    bool isPseudo() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const FieldImpl* m_pImpl;
};

} // namespace commsdsl
