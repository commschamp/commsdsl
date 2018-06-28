#pragma once

#include "commsdsl/SetField.h"
#include "commsdsl/Protocol.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class SetField : public Field
{
    using Base = Field;
public:
    SetField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const override;
    virtual std::string getCompareToValueImpl(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride) const;
    virtual std::string getCompareToFieldImpl(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride) const;

private:
    using StringsList = common::StringsList;

    std::string getExtraDoc() const;
    std::string getFieldBaseParams() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getBitsAccess() const;
    std::string getValid() const;
    void checkLengthOpt(StringsList& list) const;
    void checkDefaultValueOpt(StringsList& list) const;
    void checkReservedBitsOpt(StringsList& list) const;

    commsdsl::SetField setFieldDslObj() const
    {
        return commsdsl::SetField(dslObj());
    }

};

inline
FieldPtr createSetField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<SetField>(generator, field);
}

} // namespace commsdsl2comms
