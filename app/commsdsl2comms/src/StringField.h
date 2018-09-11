#pragma once

#include "commsdsl/StringField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class StringField : public Field
{
    using Base = Field;
public:
    StringField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual bool prepareImpl() override;
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const override;
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
    virtual std::string getPrivateRefreshBodyImpl(const FieldsList& fields) const override;
    virtual bool hasCustomReadRefreshImpl() const override;
    virtual std::string getReadPreparationImpl(const FieldsList& fields) const override;
    virtual bool isLimitedCustomizableImpl() const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldOpts(const std::string& scope) const;
    std::string getConstructor(const std::string& className) const;
    std::string getPrefixField(const std::string& scope) const;
    void checkFixedLengthOpt(StringsList& list) const;
    void checkPrefixOpt(StringsList& list) const;
    void checkSuffixOpt(StringsList& list) const;
    void checkForcingOpt(StringsList& list) const;


    commsdsl::StringField stringFieldDslObj() const
    {
        return commsdsl::StringField(dslObj());
    }

    FieldPtr m_prefix;
};

inline
FieldPtr createStringField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<StringField>(generator, field);
}

} // namespace commsdsl2comms
