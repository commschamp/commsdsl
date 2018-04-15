#pragma once

#include "Field.h"

namespace bbmp
{

class DataFieldImpl;
class BBMP_API DataField : public Field
{
    using Base = Field;
public:

    explicit DataField(const DataFieldImpl* impl);
    explicit DataField(Field field);

    const std::string& defaultValue() const;
    unsigned fixedLength() const;
    bool hasLengthPrefixField() const;
    Field lengthPrefixField() const;
};

} // namespace bbmp
