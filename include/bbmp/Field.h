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

    explicit Field(const FieldImpl* impl);
    Field(const Field& other);
    ~Field();

    const std::string& name() const;
private:
    const FieldImpl* m_pImpl;
};

} // namespace bbmp
