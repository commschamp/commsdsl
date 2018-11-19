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

#include "commsdsl/FloatField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class FloatField : public Field
{
    using Base = Field;
public:
    FloatField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override final;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override final;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override final;

private:
    using StringsList = common::StringsList;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getConstructor() const;
    std::string getSpecials() const;
    std::string getValid() const;
    StringsList getVersionBasedConditions() const;
    StringsList getNormalConditions() const;
    void checkUnitsOpt(StringsList& list) const;
    void checkVersionOpt(StringsList& list) const;
    void checkValidityOpt(StringsList& list) const;

    commsdsl::FloatField floatFieldDslObj() const
    {
        return commsdsl::FloatField(dslObj());
    }
};

inline
FieldPtr createFloatField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<FloatField>(generator, field);
}

} // namespace commsdsl2comms
