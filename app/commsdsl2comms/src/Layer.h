//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
    using LayersList = std::vector<Ptr>;

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
    void updatePluginIncludes(IncludesList& includes) const;

    bool prepare();

    std::string getClassDefinition(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const;

    static Ptr create(Generator& generator, commsdsl::Layer dslObj);

    std::string getDefaultOptions(const std::string& scope) const;
    std::string getBareMetalDefaultOptions(const std::string& scope) const;

    bool rearange(LayersList& layers, bool& success)
    {
        return rearangeImpl(layers, success);
    }

    std::string getFieldScopeForPlugin(const std::string& scope) const;
    std::string getFieldAccNameForPlugin() const;
    std::string getPluginCreatePropsFunc(const std::string& scope) const;

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
    std::string getExtraOpt(const std::string& scope) const;

    FieldPtr& memberField()
    {
        return m_field;
    }

    void setFieldForcedFailOnInvalid();
    void setFieldForcedPseudo();


    virtual bool prepareImpl();
    virtual void updateIncludesImpl(IncludesList& includes) const;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const = 0;
    virtual const std::string& getDefaultOptionStrImpl() const;
    virtual const std::string& getBareMetalOptionStrImpl() const;
    virtual bool rearangeImpl(LayersList& layers, bool& success);
    virtual bool isCustomizableImpl() const;

private:

    using GetFieldOptionsFunc = std::string (Field::*)(const std::string& scope) const;
    using GetOptionStrFunc = const std::string& (Layer::*)() const;

    bool isCustomizable() const
    {
        return isCustomizableImpl();
    }

    std::string extraOpsForExternalField() const;
    std::string getOptions(
        const std::string& scope,
        GetFieldOptionsFunc fieldFunc,
        GetOptionStrFunc optionStrFunc) const;
    const std::string& getDefaultOptionStr() const;
    const std::string& getBareMetalDefaultOptionStr() const;

    Generator& m_generator;
    commsdsl::Layer m_dslObj;
    FieldPtr m_field;
    bool m_forcedFieldFailOnInvalid = false;
    bool m_forcedFieldPseudo = false;
};

using LayerPtr = Layer::Ptr;

} // namespace commsdsl2comms
