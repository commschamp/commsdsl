#pragma once

#include "commsdsl/BitfieldField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class BitfieldField : public Field
{
    using Base = Field;
public:
    BitfieldField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

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

    std::string getFieldBaseParams() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getMembersDef(const std::string& scope) const;
    std::string getAccess() const;

    commsdsl::BitfieldField bitfieldFieldDslObj() const
    {
        return commsdsl::BitfieldField(dslObj());
    }

    std::vector<FieldPtr> m_members;
};

inline
FieldPtr createBitfieldField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<BitfieldField>(generator, field);
}

} // namespace commsdsl2comms
