#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "CommsdslApi.h"
#include "Field.h"
#include "Schema.h"

namespace commsdsl
{

class MessageImpl;
class COMMSDSL_API Message
{
public:
    using FieldsList = std::vector<Field>;
    using AttributesMap = Schema::AttributesMap;
    using ElementsList = Schema::ElementsList;
    using PlatformsList = std::vector<std::string>;

    explicit Message(const MessageImpl* impl);
    Message(const Message& other);
    ~Message();

    bool valid() const;
    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;
    std::uintmax_t id() const;
    unsigned order() const;
    std::size_t minLength() const;
    std::size_t maxLength() const;
    unsigned sinceVersion() const;
    unsigned deprecatedSince() const;
    bool isDeprecatedRemoved() const;
    FieldsList fields() const;
    std::string externalRef() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;
    const PlatformsList& platforms() const;

protected:
    const MessageImpl* m_pImpl;
};

} // namespace commsdsl
