#pragma once

#include <string>
#include <memory>
#include <vector>

#include "commsdsl/Layer.h"
#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class Generator;
class Layer
{
public:
    using Ptr = std::unique_ptr<Layer>;
//    using LayersList = std::vector<Ptr>;

    virtual ~Layer() = default;

    const std::string& name() const
    {
        return m_dslObj.name();
    }

    commsdsl::Layer::Kind kind() const
    {
        return m_dslObj.kind();
    }

    using IncludesList = common::StringsList;
    void updateIncludes(IncludesList& includes) const;

    bool prepare();

    std::string getClassDefinition(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const;

    static Ptr create(Generator& generator, commsdsl::Layer dslObj);

    std::string getDefaultOptions(const std::string& scope) const;

//    bool writeProtocolDefinition() const;

//    const std::string& getDisplayName() const;

//    std::string getClassPrefix(
//            const std::string& suffix,
//            bool checkForOptional = true,
//            const std::string& extraDoc = common::emptyString()) const;

protected:
    Layer(Generator& generator, commsdsl::Layer field)
      : m_generator(generator),
        m_dslObj(field) {}

    Generator& generator() const
    {
        return m_generator;
    }

    const commsdsl::Layer& dslObj() const
    {
        return m_dslObj;
    }

    const Field* getField() const;

    std::string getPrefix() const;
    std::string getFieldDefinition(const std::string& scope) const;
    std::string getFieldType() const;

    FieldPtr& memberField()
    {
        return m_field;
    }

    void setFieldForcedFailOnInvalid();

    virtual bool prepareImpl();
    virtual void updateIncludesImpl(IncludesList& includes) const;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const = 0;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const;

private:

    Generator& m_generator;
    commsdsl::Layer m_dslObj;
    FieldPtr m_field;
    bool m_forcedFieldFailOnInvalid = false;
};

using LayerPtr = Layer::Ptr;

} // namespace commsdsl2comms
