#pragma once

#include "commsdsl/IntField.h"

#include "Field.h"

namespace commsdsl2comms
{

class IntField : public Field
{
    using Base = Field;
public:
    IntField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual const IncludesList& extraIncludesImpl() const override;
};

inline
FieldPtr createIntField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<IntField>(generator, field);
}

} // namespace commsdsl2comms
