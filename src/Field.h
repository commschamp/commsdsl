#pragma once

#include <string>
#include <memory>
#include <vector>

#include "commsdsl/Field.h"
#include "common.h"

namespace commsdsl2comms
{

class Generator;
class Field
{
public:
    using Ptr = std::unique_ptr<Field>;

    const std::string& name() const
    {
        return m_dslObj.name();
    }

    std::size_t minLength() const
    {
        return m_dslObj.minLength();
    }

    std::size_t maxLength() const
    {
        return m_dslObj.maxLength();
    }

    using IncludesList = common::StringsList;
    void updateIncludes(IncludesList& includes) const;

    bool doesExist() const;

    bool prepare();

    std::string getClassDefinition(const std::string& scope) const;

    static Ptr create(Generator& generator, commsdsl::Field dslObj);

    std::string getDefaultOptions() const;

    commsdsl::Field::SemanticType semanticType() const
    {
        return m_dslObj.semanticType();
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

    const std::string& externalRef() const
    {
        return m_externalRef;
    }

    virtual bool prepareImpl();
    virtual const IncludesList& extraIncludesImpl() const;
    virtual std::string getClassDefinitionImpl(const std::string& scope) const = 0;

    const std::string& getDisplayName() const;
    std::string getNameFunc() const;

    bool hasExtraOptions(const std::string& scope) const
    {
        return (!scope.empty()) || (!m_externalRef.empty());
    }

private:
    Generator& m_generator;
    commsdsl::Field m_dslObj;
    std::string m_externalRef;
};

using FieldPtr = Field::Ptr;

} // namespace commsdsl2comms
