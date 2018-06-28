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

    static const std::string& convertType(commsdsl::IntField::Type value, std::size_t len = 0);
protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const override;
    virtual std::string getCompareToValueImpl(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride,
        bool forcedVersionOptional) const override;
    virtual std::string getCompareToFieldImpl(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride,
        bool forcedVersionOptional) const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getFieldChangedSignType() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getSpecials() const;
    std::string getValid() const;
    void checkDefaultValueOpt(StringsList& list) const;
    void checkLengthOpt(StringsList& list) const;
    void checkSerOffsetOpt(StringsList& list) const;
    void checkScalingOpt(StringsList& list) const;
    void checkUnitsOpt(StringsList& list) const;
    void checkValidRangesOpt(StringsList& list) const;

    bool isUnsigned() const;


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
