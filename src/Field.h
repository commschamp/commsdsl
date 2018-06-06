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
        return m_dslObj.name();
    }

    std::size_t minLength() const
    {
        return m_dslObj.minLength();
    }

    std::size_t maxLength() const
    {
        return m_dslObj.maxLength();
    }

    using IncludesList = common::IncludesList;
    void updateIncludes(IncludesList& includes) const;

    bool doesExist() const;

    bool prepare()
    {
        return prepareImpl();
    }

    static Ptr create(Generator& generator, commsdsl::Field dslObj);

protected:
    Field(Generator& generator, commsdsl::Field field)
      : m_generator(generator),
        m_dslObj(field) {}

    Generator& generator() const
    {
        return m_generator;
    }

    const commsdsl::Field& dslObj() const
    {
        return m_dslObj;
    }

    virtual bool prepareImpl();
    virtual const IncludesList& extraIncludesImpl() const;

private:
    Generator& m_generator;
    commsdsl::Field m_dslObj;
};

using FieldPtr = Field::Ptr;

} // namespace commsdsl2comms
