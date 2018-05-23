#pragma once

#include "Field.h"

namespace commsdsl
{

class RefFieldImpl;
class COMMSDSL_API RefField : public Field
{
    using Base = Field;
public:

    explicit RefField(const RefFieldImpl* impl);
    explicit RefField(Field field);

    Field field() const;
};

} // namespace commsdsl
