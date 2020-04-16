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
    virtual void updateIncludesCommonImpl(IncludesList& includes) const override final;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override final;
    virtual std::string getExtraDefaultOptionsImpl(const std::string& scope) const override final;
    virtual std::string getExtraBareMetalDefaultOptionsImpl(const std::string& base, const std::string& scope) const override final;
    virtual std::string getPluginAnonNamespaceImpl(
        const std::string& scope,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const override final;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override final;
    virtual void setForcedPseudoImpl() override final;
    virtual void setForcedNoOptionsConfigImpl() override final;
    virtual bool isVersionDependentImpl() const override final;
    virtual std::string getCommonDefinitionImpl(const std::string& fullScope) const override final;
    virtual std::string getExtraRefToCommonDefinitionImpl(const std::string& fullScope) const override final;
    virtual bool verifyAliasImpl(const std::string& fieldName) const override final;

private:
    using StringsList = common::StringsList;
    using GetExtraOptionsFunc = std::string (Field::*)(const std::string& base, const std::string& scope) const;

    std::string getFieldBaseParams() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getMembersDef(const std::string& scope) const;
    std::string getAccess() const;
    std::string getExtraOptions(const std::string& scope, GetExtraOptionsFunc func, const std::string& base) const;

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
