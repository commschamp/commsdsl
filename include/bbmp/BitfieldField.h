#pragma once

#include "Field.h"

namespace bbmp
{

class BitfieldFieldImpl;
class BBMP_API BitfieldField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;

    explicit BitfieldField(const BitfieldFieldImpl* impl);
    explicit BitfieldField(Field field);

    Endian endian() const;
    const Members& members() const;
};

} // namespace bbmp
