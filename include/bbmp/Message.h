#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "BbmpApi.h"
#include "Field.h"

namespace bbmp
{

class MessageImpl;
class BBMP_API Message
{
public:
    using FieldsList = std::vector<Field>;

    explicit Message(const MessageImpl* impl);
    Message(const Message& other);
    ~Message();

    bool valid() const;
    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;
    std::uintmax_t id() const;
    std::size_t minLength() const;
    std::size_t maxLength() const;
    unsigned sinceVersion() const;
    unsigned deprecatedSince() const;
    bool isDeprecatedRemoved() const;
    FieldsList fields() const;

protected:
    const MessageImpl* m_pImpl;
};

} // namespace bbmp
