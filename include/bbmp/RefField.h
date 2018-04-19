#pragma once

#include "Field.h"

namespace bbmp
{

class RefFieldImpl;
class BBMP_API RefField : public Field
{
    using Base = Field;
public:

    explicit RefField(const RefFieldImpl* impl);
    explicit RefField(Field field);

    Field field() const;
};

} // namespace bbmp
