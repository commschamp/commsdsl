#pragma once

#include <string>
#include <memory>
#include <vector>

#include "commsdsl/Field.h"
#include "commsdsl/Endian.h"
#include "commsdsl/OptCond.h"
#include "common.h"

namespace commsdsl2comms
{

class Generator;
class Field
{
public:
    using Ptr = std::unique_ptr<Field>;
    using FieldsList = std::vector<Ptr>;

    virtual ~Field() = default;

    const std::string& name() const
    {
        return m_dslObj.name();
    }

    const std::string& displayName() const
    {
        return m_dslObj.displayName();
    }


    commsdsl::Field::Kind kind() const
    {
        return m_dslObj.kind();
    }

    std::size_t minLength() const;

    std::size_t maxLength() const
    {
        return maxLengthImpl();
    }

    using IncludesList = common::StringsList;
    void updateIncludes(IncludesList& includes) const;

    void updatePluginIncludes(IncludesList& includes) const;

    bool doesExist() const;

    bool prepare(unsigned parentVersion);

    std::string getClassDefinition(
        const std::string& scope,
        const std::string& suffix = common::emptyString()) const;

    static Ptr create(Generator& generator, commsdsl::Field dslObj);

    std::string getDefaultOptions(const std::string& scope) const;

    commsdsl::Field::SemanticType semanticType() const
    {
        return m_dslObj.semanticType();
    }

    bool writeFiles() const;

    const std::string& getDisplayName() const;

    std::string getClassPrefix(
            const std::string& suffix,
            bool checkForOptional = true,
            const std::string& extraDoc = common::emptyString()) const;

    const std::string& externalRef() const
    {
        return m_externalRef;
    }

    static std::string dslCondToString(
        const FieldsList& fields,
        const commsdsl::OptCond& cond,
        bool bracketsWrap = false);

    std::string getCompareToValue(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride = common::emptyString(),
        bool forcedVersionOptional = false) const
    {
        return getCompareToValueImpl(op, value, nameOverride, forcedVersionOptional);
    }

    std::string getCompareToField(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride = common::emptyString(),
        bool forcedVersionOptional = false) const
    {
        return getCompareToFieldImpl(op, field, nameOverride, forcedVersionOptional);
    }

    bool isVersionOptional() const;

    void setForcedFailOnInvalid()
    {
        m_focedFailOnInvalid = true;
    }

    static std::string getReadForFields(const FieldsList& fields, bool forMessage);
    static std::string getPublicRefreshForFields(const FieldsList& fields, bool forMessage);
    static std::string getPrivateRefreshForFields(const FieldsList& fields);

    std::string getPluginCreatePropsFunc(const std::string& scope,
        bool forcedSerialisedHidden,
        bool serHiddenParam = true) const;

    std::string getPluginAnonNamespace(
        const std::string& scope = common::emptyString(),
        bool forcedSerialisedHidden = false,
        bool serHiddenParam = true) const;
    std::string getPluginProperties(bool serHiddenParam) const
    {
        return getPluginPropertiesImpl(serHiddenParam);
    }

protected:
    Field(Generator& generator, commsdsl::Field field)
      : m_generator(generator),
        m_dslObj(field) {}

    Generator& generator() const
    {
        return m_generator;
    }

    const commsdsl::Field& dslObj() const
    {
        return m_dslObj;
    }

    virtual bool prepareImpl();
    virtual void updateIncludesImpl(IncludesList& includes) const;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const;
    virtual std::size_t minLengthImpl() const;
    virtual std::size_t maxLengthImpl() const;
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const = 0;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const;
    virtual std::string getCompareToValueImpl(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride,
        bool forcedVersionOptional) const;
    virtual std::string getCompareToFieldImpl(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride,
        bool forcedVersionOptional) const;
    virtual std::string getPluginPropsDefFuncBodyImpl(
        const std::string& scope,
        bool externalName,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const;
    virtual std::string getPluginAnonNamespaceImpl(
        const std::string& scope,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const;

    std::string getNameFunc() const;

    void updateExtraOptions(const std::string& scope, common::StringsList& options) const;

    std::string getCustomRead() const;
    std::string getCustomWrite() const;
    std::string getCustomLength() const;
    std::string getCustomValid() const;
    std::string getCustomRefresh() const;
    std::string getCommonFieldBaseParams(commsdsl::Endian endian = commsdsl::Endian_NumOfValues) const;

private:

    bool writeProtocolDefinitionFile() const;
    bool writePluginHeaderFile() const;
    bool writePluginScrFile() const;

    std::string getPluginIncludes() const;

    Generator& m_generator;
    commsdsl::Field m_dslObj;
    std::string m_externalRef;
    unsigned m_parentVersion = 0U;
    bool m_focedFailOnInvalid = false;
};

using FieldPtr = Field::Ptr;

} // namespace commsdsl2comms
