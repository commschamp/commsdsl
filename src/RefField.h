#pragma once

#include "commsdsl/RefField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class RefField : public Field
{
    using Base = Field;
public:
    RefField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual bool prepareImpl() override;
    virtual const IncludesList& extraIncludesImpl() const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope) const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getOpts(const std::string& scope) const;

    commsdsl::RefField refFieldDslObj() const
    {
        return commsdsl::RefField(dslObj());
    }

private:
    IncludesList m_includes;
};

inline
FieldPtr createRefField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<RefField>(generator, field);
}

} // namespace commsdsl2comms
