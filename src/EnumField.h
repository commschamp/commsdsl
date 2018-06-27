#pragma once

#include "commsdsl/EnumField.h"
#include "commsdsl/Protocol.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class EnumField : public Field
{
    using Base = Field;
public:
    EnumField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

    common::StringsList getValuesList() const;
    std::string getValuesDefinition() const;
    std::string getValueName(std::intmax_t value) const;
    const std::string& underlyingType() const;

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

    struct RangeInfo
    {
        std::intmax_t m_min = 0;
        std::intmax_t m_max = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = commsdsl::Protocol::notYetDeprecated();
    };

    using ValidRangesList = std::vector<RangeInfo>;

    std::string getEnumeration() const;
    std::string getFieldBaseParams() const;
    std::string getEnumType(const std::string& suffix) const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getValid() const;
    void checkDefaultValueOpt(StringsList& list) const;
    void checkValidRangesOpt(StringsList& list) const;
    bool prepareRanges() const;

    commsdsl::EnumField enumFieldDslObj() const
    {
        return commsdsl::EnumField(dslObj());
    }

    mutable ValidRangesList m_validRanges;
};

inline
FieldPtr createEnumField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<EnumField>(generator, field);
}

} // namespace commsdsl2comms
