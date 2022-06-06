//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/RefField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2old
{

class RefField final : public Field
{
    using Base = Field;
public:
    RefField(Generator& generator, commsdsl::parse::Field field) : Base(generator, field) {}

protected:
    virtual bool prepareImpl() override;
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual void updateIncludesCommonImpl(IncludesList& includes) const override;
    virtual void updatePluginIncludesImpl(IncludesList& includes) const override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override;
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
    virtual std::string getPluginPropsDefFuncBodyImpl(
        const std::string& scope,
        bool externalName,
        bool forcedSerialisedHidden,
        bool serHiddenParam) const override;
    virtual std::string getCommonDefinitionImpl(const std::string& fullScope) const override;
    virtual std::string getExtraRefToCommonDefinitionImpl(const std::string& fullScope) const override;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getOpts(const std::string& scope) const;
    std::string getPropsUpdate() const;

    commsdsl::parse::RefField refFieldDslObj() const
    {
        return commsdsl::parse::RefField(dslObj());
    }
};

inline
FieldPtr createRefField(Generator& generator, commsdsl::parse::Field field)
{
    return std::make_unique<RefField>(generator, field);
}

} // namespace commsdsl2old