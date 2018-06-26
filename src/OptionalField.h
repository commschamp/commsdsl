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

protected:
    virtual bool prepareImpl() override;
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const override;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const override;

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
