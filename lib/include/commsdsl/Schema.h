#pragma once

#include <string>
#include <map>
#include <vector>

#include "CommsdslApi.h"
#include "Endian.h"

namespace commsdsl
{

class SchemaImpl;
class COMMSDSL_API Schema
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

    bool nonUniqueMsgIdAllowed() const;

    const AttributesMap& extraAttributes() const;

    const ElementsList& extraElements() const;

private:
    const SchemaImpl* m_pImpl;
};

} // namespace commsdsl
