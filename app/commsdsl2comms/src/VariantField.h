#pragma once

#include "commsdsl/VariantField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class VariantField : public Field
{
    using Base = Field;
public:
    VariantField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual bool prepareImpl() override final;
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override final;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const override final;
    virtual std::string getPluginAnonNamespaceImpl(
        const std::string& scope,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const override final;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override final;

private:
    using StringsList = common::StringsList;

    std::string getFieldOpts(const std::string& scope) const;
    std::string getMembersDef(const std::string& scope) const;
    std::string getAccess() const;
    std::string getRead() const;
    std::string getRefresh() const;
    std::string getPrivate() const;

    commsdsl::VariantField variantFieldDslObj() const
    {
        return commsdsl::VariantField(dslObj());
    }

    std::vector<FieldPtr> m_members;
};

inline
FieldPtr createVariantField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<VariantField>(generator, field);
}

} // namespace commsdsl2comms
