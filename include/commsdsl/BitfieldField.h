#pragma once

#include "Field.h"

namespace commsdsl
{

class BitfieldFieldImpl;
class COMMSDSL_API BitfieldField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;

    explicit BitfieldField(const BitfieldFieldImpl* impl);
    explicit BitfieldField(Field field);

    Endian endian() const;
    Members members() const;
};

} // namespace commsdsl
