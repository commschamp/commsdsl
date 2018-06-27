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
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const override;
    virtual std::string getCompareToValueImpl(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride) const override;
    virtual std::string getCompareToFieldImpl(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride) const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getOpts(const std::string& scope) const;

    commsdsl::RefField refFieldDslObj() const
    {
        return commsdsl::RefField(dslObj());
    }
};

inline
FieldPtr createRefField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<RefField>(generator, field);
}

} // namespace commsdsl2comms
