//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/SetField.h"
#include "commsdsl/Protocol.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class SetField : public Field
{
    using Base = Field;
public:
    SetField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual void updateIncludesCommonImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override final;
    virtual std::string getCompareToValueImpl(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride,
        bool forcedVersionOptional) const override final;
    virtual std::string getCompareToFieldImpl(
        const std::string& op,
        const Field& field,
        const std::string& nameOverride,
        bool forcedVersionOptional) const override final;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override final;
    virtual std::string getCommonDefinitionImpl(const std::string& fullScope) const override final;
    virtual bool hasCommonDefinitionImpl() const override final;

private:
    using StringsList = common::StringsList;

    std::string getExtraDoc() const;
    std::string getFieldBaseParams() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getBitsAccess() const;
    std::string getValid() const;
    std::string getBitName(const std::string& fullScope) const;
    std::string getBitNameWrap(const std::string& scope) const;
    void checkLengthOpt(StringsList& list) const;
    void checkDefaultValueOpt(StringsList& list) const;
    void checkReservedBitsOpt(StringsList& list) const;

    commsdsl::SetField setFieldDslObj() const
    {
        return commsdsl::SetField(dslObj());
    }

};

inline
FieldPtr createSetField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<SetField>(generator, field);
}

} // namespace commsdsl2comms
