#pragma once

#include "Field.h"

namespace bbmp
{

class BitfieldFieldImpl;
class BitfieldField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;

    explicit BitfieldField(const BitfieldFieldImpl* impl);
    explicit BitfieldField(Field field);

    Endian endian() const;
    std::size_t length() const;
    std::size_t bitLength() const;
    const Members& members() const;
};

} // namespace bbmp
