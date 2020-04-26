//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

    const std::string& displayName() const;


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
    void updateIncludesCommon(IncludesList& includes) const;

    void updatePluginIncludes(IncludesList& includes) const;

    bool doesExist() const;

    bool prepare(unsigned parentVersion);

    std::string getClassDefinition(
        const std::string& scope,
        const std::string& className = common::emptyString()) const;

    std::string getCommonDefinition(const std::string& scope) const;
    std::string getExtraRefToCommonDefinition(const std::string& fullScope) const
    {
        return getExtraRefToCommonDefinitionImpl(fullScope);
    }

    static Ptr create(Generator& generator, commsdsl::Field dslObj);

    std::string getDefaultOptions(const std::string& base, const std::string& scope) const;
    std::string getBareMetalDefaultOptions(const std::string& base, const std::string& scope) const;
    std::string getDataViewDefaultOptions(const std::string& base, const std::string& scope) const;

    commsdsl::Field::SemanticType semanticType() const
    {
        return m_dslObj.semanticType();
    }

    bool writeFiles() const;

    std::string getClassPrefix(
            const std::string& suffix,
            bool checkForOptional = true,
            const std::string& extraDetails = common::emptyString(),
            const std::string& extraDoxygen = common::emptyString()) const;

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
    bool isVersionDependent() const;

    unsigned sinceVersion() const
    {
        return m_dslObj.sinceVersion();
    }

    void setForcedFailOnInvalid()
    {
        m_focedFailOnInvalid = true;
    }

    void setForcedPseudo()
    {
        m_forcedPseudo = true;
        setForcedPseudoImpl();
    }

    void setForcedNoOptionsConfig()
    {
        m_forcedNoOptionsConfig = true;
        setForcedNoOptionsConfigImpl();
    }

    void setMemberChild()
    {
        m_memberChild = true;
    }

    bool isPseudo() const;
    bool isForceGen() const;

    static std::string getReadForFields(
        const FieldsList& fields,
        bool forMessage = false,
        bool updateVersion = false);
    static std::string getPublicRefreshForFields(
        const FieldsList& fields, 
        bool forMessage = false);
    static std::string getPrivateRefreshForFields(const FieldsList& fields);

    std::string getPrivateRefreshBody(const FieldsList& fields) const
    {
        return getPrivateRefreshBodyImpl(fields);
    }

    bool hasCustomReadRefresh() const
    {
        return hasCustomReadRefreshImpl();
    }

    std::string getReadPreparation(const FieldsList& fields) const
    {
        return getReadPreparationImpl(fields);
    }

    std::string getPluginCreatePropsFunc(
        const std::string& scope,
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

    const commsdsl::Field& dslObj() const
    {
        return m_dslObj;
    }

    bool verifyAlias(const std::string& fieldName) const;

protected:
    Field(Generator& generator, commsdsl::Field field)
      : m_generator(generator),
        m_dslObj(field) {}

    Generator& generator() const
    {
        return m_generator;
    }

    bool isForcedNoOptionsConfig() const
    {
        return m_forcedNoOptionsConfig;
    }

    bool isMemberChild() const
    {
        return m_memberChild;
    }

    virtual bool prepareImpl();
    virtual void updateIncludesImpl(IncludesList& includes) const;
    virtual void updateIncludesCommonImpl(IncludesList& includes) const;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const;
    virtual std::size_t minLengthImpl() const;
    virtual std::size_t maxLengthImpl() const;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const = 0;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const;
    virtual std::string getExtraBareMetalDefaultOptionsImpl(const std::string& base, const std::string& scope) const;
    virtual std::string getExtraDataViewDefaultOptionsImpl(const std::string& base, const std::string& scope) const;
    virtual std::string getBareMetalOptionStrImpl() const;
    virtual std::string getDataViewOptionStrImpl() const;
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
    virtual std::string getPrivateRefreshBodyImpl(const FieldsList& fields) const;
    virtual bool hasCustomReadRefreshImpl() const;
    virtual std::string getReadPreparationImpl(const FieldsList& fields) const;
    virtual bool isLimitedCustomizableImpl() const;
    virtual void setForcedPseudoImpl();
    virtual void setForcedNoOptionsConfigImpl();
    virtual bool isVersionDependentImpl() const;
    virtual std::string getCommonDefinitionImpl(const std::string& fullScope) const;
    virtual std::string getExtraRefToCommonDefinitionImpl(const std::string& fullScope) const;
    virtual bool verifyAliasImpl(const std::string& fieldName) const;

    std::string getNameCommonWrapFunc(const std::string& scope) const;
    std::string getCommonNameFunc(const std::string& fullScope) const;

    void updateExtraOptions(
        const std::string& scope,
        common::StringsList& options,
        bool ignoreFailOnInvalid = false) const;

    const std::string& getCustomRead() const;
    const std::string& getCustomWrite() const;
    std::string getCustomLength() const;
    std::string getCustomValid() const;
    const std::string& getCustomRefresh() const;
    std::string getExtraPublic() const;
    std::string getExtraProtected() const;
    std::string getFullProtected() const;
    std::string getExtraPrivate() const;
    std::string getFullPrivate() const;
    std::string getCommonFieldBaseParams(commsdsl::Endian endian = commsdsl::Endian_NumOfValues) const;

    bool isCustomizable() const;

    std::string adjustScopeWithNamespace(const std::string& scope) const;
    std::string scopeForCommon(const std::string& scope) const;
    std::string classNameFromFullScope(const std::string& fullScope) const;

private:

    bool writeProtocolDefinitionCommonFile() const;
    bool writeProtocolDefinitionFile() const;
    bool writePluginHeaderFile() const;
    bool writePluginScrFile() const;

    std::string getPluginIncludes() const;

    Generator& m_generator;
    commsdsl::Field m_dslObj;
    std::string m_externalRef;
    unsigned m_parentVersion = 0U;
    std::string m_customRead;
    std::string m_customRefresh;
    std::string m_customWrite;
    bool m_focedFailOnInvalid = false;
    bool m_forcedPseudo = false;
    bool m_forcedNoOptionsConfig = false;
    bool m_memberChild = false;
};

using FieldPtr = Field::Ptr;

} // namespace commsdsl2comms
