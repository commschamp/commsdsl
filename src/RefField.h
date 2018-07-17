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
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
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
    virtual std::string getPluginPropsDefFuncBodyImpl(
        const std::string& scope,
        bool externalName) const override;


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
