#pragma once

#include <vector>

#include "Field.h"

namespace bbmp
{

class OptionalFieldImpl;
class BBMP_API OptionalField : public Field
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
};

} // namespace bbmp
