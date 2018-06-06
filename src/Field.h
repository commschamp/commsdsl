#pragma once

#include <string>
#include <memory>
#include <vector>

#include "commsdsl/Field.h"
#include "common.h"

namespace commsdsl2comms
{

class Generator;
class Field
{
public:
    using Ptr = std::unique_ptr<Field>;

    const std::string& name() const
    {
        return m_field.name();
    }

    using IncludesList = common::IncludesList;
    void updateIncludes(IncludesList& includes) const;

    bool prepare()
    {
        return prepareImpl();
    }

    static Ptr create(Generator& generator, commsdsl::Field field);

protected:
    Field(Generator& generator, commsdsl::Field field)
      : m_generator(generator),
        m_field(field) {}

    Generator& generator() const
    {
        return m_generator;
    }

    const commsdsl::Field& field() const
    {
        return m_field;
    }

    virtual bool prepareImpl();
    virtual const IncludesList& extraIncludesImpl() const;

private:
    Generator& m_generator;
    commsdsl::Field m_field;
};

using FieldPtr = Field::Ptr;

} // namespace commsdsl2comms
