#pragma once

#include <string>
#include <map>
#include <vector>

#include "Endian.h"

namespace bbmp
{

class SchemaImpl;
class Schema
{
public:
    using AttributesMap = std::multimap<std::string, std::string>;
    using ElementsList = std::vector<std::string>;

    explicit Schema(const SchemaImpl* impl);

    bool valid() const;

    const std::string& name() const;

    const std::string& description() const;

    unsigned id() const;

    unsigned version() const;

    Endian endian() const;

    const AttributesMap& unknownAttributes() const;

    const ElementsList& unknownElements() const;

private:
    const SchemaImpl* m_pImpl;
};

} // namespace bbmp
