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

    const std::string& name() const
    {
        return m_dslObj.name();
    }

    commsdsl::Field::Kind kind() const
    {
        return m_dslObj.kind();
    }

    std::size_t minLength() const;

    std::size_t maxLength() const
    {
        return m_dslObj.maxLength();
    }

    using IncludesList = common::StringsList;
    void updateIncludes(IncludesList& includes) const;

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

    bool writeProtocolDefinition() const;

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
        const std::string& nameOverride = common::emptyString()) const
    {
        return getCompareToValueImpl(op, value, nameOverride);
    }

    std::string getCompareToField(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride = common::emptyString()) const
    {
        return getCompareToFieldImpl(op, field, nameOverride);
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
    virtual std::string getClassDefinitionImpl(const std::string& scope, const std::string& suffix) const = 0;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const;
    virtual std::string getCompareToValueImpl(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride) const;
    virtual std::string getCompareToFieldImpl(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride) const;

    std::string getNameFunc() const;

    void updateExtraOptions(const std::string& scope, common::StringsList& options) const;

    std::string getCustomRead() const;
    std::string getCustomWrite() const;
    std::string getCustomLength() const;
    std::string getCustomValid() const;
    std::string getCustomRefresh() const;
    std::string getCommonFieldBaseParams(commsdsl::Endian endian = commsdsl::Endian_NumOfValues) const;

private:

    bool isVersionOptional() const;

    Generator& m_generator;
    commsdsl::Field m_dslObj;
    std::string m_externalRef;
    unsigned m_parentVersion = 0U;
};

using FieldPtr = Field::Ptr;

} // namespace commsdsl2comms
