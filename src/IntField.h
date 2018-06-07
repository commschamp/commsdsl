#pragma once

#include "commsdsl/IntField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class IntField : public Field
{
    using Base = Field;
public:
    IntField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual const IncludesList& extraIncludesImpl() const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope) const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getName() const;
    void checkLengthOpt(StringsList& list) const;

    commsdsl::IntField intFieldDslObj() const
    {
        return commsdsl::IntField(dslObj());
    }
};

inline
FieldPtr createIntField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<IntField>(generator, field);
}

} // namespace commsdsl2comms
