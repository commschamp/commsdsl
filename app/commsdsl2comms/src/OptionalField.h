#pragma once

#include "commsdsl/OptionalField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class OptionalField : public Field
{
    using Base = Field;
public:
    OptionalField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

    commsdsl::OptCond cond() const
    {
        return optionalFieldDslObj().cond();
    }
protected:
    virtual bool prepareImpl() override;
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const override;
    virtual std::string getPluginAnonNamespaceImpl(
        const std::string& scope,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const override;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override;
    virtual std::string getPrivateRefreshBodyImpl(const FieldsList& fields) const override;
    virtual bool requiresReadPreparationImpl() const override;
    virtual std::string getReadPreparationImpl() const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldOpts(const std::string& scope) const;
    std::string getMembersDef(const std::string& scope) const;
    std::string getFieldRef() const;
    void checkModeOpt(StringsList& options) const;

    commsdsl::OptionalField optionalFieldDslObj() const
    {
        return commsdsl::OptionalField(dslObj());
    }

    FieldPtr m_field;
};

inline
FieldPtr createOptionalField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<OptionalField>(generator, field);
}

} // namespace commsdsl2comms
