#pragma once

#include "commsdsl/ListField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class ListField : public Field
{
    using Base = Field;
public:
    ListField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual bool prepareImpl() override;
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override;
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
    virtual std::string getPluginAnonNamespaceImpl(
        const std::string& scope,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const override;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override;
    virtual std::string getPrivateRefreshBodyImpl(const FieldsList& fields) const override;
    virtual bool hasCustomReadRefreshImpl() const override;
    virtual std::string getReadPreparationImpl(const FieldsList& fields) const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldOpts(const std::string& scope) const;
    std::string getElement() const;
    std::string getMembersDef(const std::string& scope) const;
    void checkFixedSizeOpt(StringsList& list) const;
    void checkCountPrefixOpt(StringsList& list) const;
    void checkLengthPrefixOpt(StringsList& list) const;
    void checkElemLengthPrefixOpt(StringsList& list) const;
    bool checkDetachedPrefixOpt(StringsList& list) const;
    bool isElemForcedSerialisedHiddenInPlugin() const;
    std::string getPrefixName() const;


    commsdsl::ListField listFieldDslObj() const
    {
        return commsdsl::ListField(dslObj());
    }

    FieldPtr m_element;
    FieldPtr m_countPrefix;
    FieldPtr m_lengthPrefix;
    FieldPtr m_elemLengthPrefix;
};

inline
FieldPtr createListField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<ListField>(generator, field);
}

} // namespace commsdsl2comms
