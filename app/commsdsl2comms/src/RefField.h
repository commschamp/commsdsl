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
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override final;
    virtual std::string getCompareToValueImpl(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride,
        bool forcedVersionOptional) const override final;
    virtual std::string getCompareToFieldImpl(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride,
        bool forcedVersionOptional) const override final;
    virtual std::string getPluginPropsDefFuncBodyImpl(
        const std::string& scope,
        bool externalName,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const override final;

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
