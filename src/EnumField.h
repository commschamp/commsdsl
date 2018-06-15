#pragma once

#include "commsdsl/EnumField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class EnumField : public Field
{
    using Base = Field;
public:
    EnumField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

    common::StringsList getValuesList(bool description = true) const;
    std::string getValuesDefinition() const;
    std::string getValueName(std::intmax_t value) const;

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const override;

private:
    using StringsList = common::StringsList;

    std::string getEnumeration() const;
    std::string getFieldBaseParams() const;
    std::string getEnumType(const std::string& suffix) const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getSpecials() const;
    std::string getValid() const;
    void checkDefaultValueOpt(StringsList& list) const;
    void checkValidRangesOpt(StringsList& list) const;

    commsdsl::EnumField enumFieldDslObj() const
    {
        return commsdsl::EnumField(dslObj());
    }
};

inline
FieldPtr createEnumField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<EnumField>(generator, field);
}

} // namespace commsdsl2comms
