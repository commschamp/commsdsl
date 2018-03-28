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

    enum class Kind
    {
        Int,
        Float,
        Bitfield,
        Bundle,
        Ref,
        NumOfValues
    };

    explicit Field(const FieldImpl* impl);
    Field(const Field& other);
    ~Field();

    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;
    Kind kind() const;

protected:
    const FieldImpl* m_pImpl;
};

} // namespace bbmp
