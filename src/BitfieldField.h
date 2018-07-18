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
    virtual bool prepareImpl() override;
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const override;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const override;
    virtual std::string getPluginAnonNamespaceImpl(
        const std::string& scope,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const override;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getMembersDef(const std::string& scope, const std::string& suffix) const;
    std::string getAccess(const std::string& suffix) const;

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
