#pragma once

#include <vector>

#include "Field.h"
#include "OptCond.h"

namespace commsdsl
{

class OptionalFieldImpl;
class COMMSDSL_API OptionalField : public Field
{
    using Base = Field;
public:

    enum class Mode
    {
        Tentative,
        Exists,
        Missing,
        NumOfValues
    };

    explicit OptionalField(const OptionalFieldImpl* impl);
    explicit OptionalField(Field field);

    Mode defaultMode() const;
    Field field() const;
    OptCond cond() const;
};

} // namespace commsdsl
