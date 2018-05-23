#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "BbmpApi.h"
#include "Field.h"

namespace bbmp
{

class InterfaceImpl;
class BBMP_API Interface
{
public:
    using FieldsList = std::vector<Field>;
    using AttributesMap = Schema::AttributesMap;
    using ElementsList = Schema::ElementsList;

    explicit Interface(const InterfaceImpl* impl);
    Interface(const Interface& other);
    ~Interface();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    FieldsList fields() const;
    std::string externalRef() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const InterfaceImpl* m_pImpl;
};

} // namespace bbmp
