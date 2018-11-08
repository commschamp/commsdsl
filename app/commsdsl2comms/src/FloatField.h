#pragma once

#include "commsdsl/FloatField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class FloatField : public Field
{
    using Base = Field;
public:
    FloatField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override final;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override final;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getConstructor() const;
    std::string getSpecials() const;
    std::string getValid() const;
    StringsList getVersionBasedConditions() const;
    StringsList getNormalConditions() const;
    void checkUnitsOpt(StringsList& list) const;
    void checkVersionOpt(StringsList& list) const;
    void checkValidityOpt(StringsList& list) const;

    commsdsl::FloatField floatFieldDslObj() const
    {
        return commsdsl::FloatField(dslObj());
    }
};

inline
FieldPtr createFloatField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<FloatField>(generator, field);
}

} // namespace commsdsl2comms
