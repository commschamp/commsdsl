#pragma once

#include <string>
#include <vector>

#include "BbmpApi.h"
#include "Field.h"

namespace bbmp
{

class NamespaceImpl;
class BBMP_API Namespace
{
public:

    using FieldsList = std::vector<Field>;

    explicit Namespace(const NamespaceImpl* impl);
    Namespace(const Namespace& other);
    ~Namespace();

    const std::string& name() const;
    const FieldsList& fields() const;

private:
    const NamespaceImpl* m_pImpl;
};

} // namespace bbmp
