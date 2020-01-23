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

#include "commsdsl/IntField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2comms
{

class IntField : public Field
{
    using Base = Field;
public:
    IntField(Generator& generator, commsdsl::Field field) : Base(generator, field) {}

    static const std::string& convertType(commsdsl::IntField::Type value, std::size_t len = 0);
    static bool isUnsignedType(commsdsl::IntField::Type value);
    bool isUnsignedType() const;
    bool isValidPropKey() const;
    std::string getPropKeyType() const;
    std::string getPropKeyValueStr() const;

protected:
    virtual bool prepareImpl() override final;
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

private:
    using StringsList = common::StringsList;
    using SpecialsListElem = std::pair<std::string, commsdsl::IntField::SpecialValueInfo>;
    using SpecialsList = std::vector<SpecialsListElem>;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getFieldChangedSignType() const;
    std::string getFieldOpts(const std::string& scope, bool reduced = false) const;
    std::string getSpecials(const std::string& scope) const;
    std::string getValid() const;
    void checkDefaultValueOpt(StringsList& list) const;
    void checkLengthOpt(StringsList& list) const;
    void checkSerOffsetOpt(StringsList& list) const;
    void checkScalingOpt(StringsList& list) const;
    void checkUnitsOpt(StringsList& list) const;
    void checkValidRangesOpt(StringsList& list) const;

    bool isUnsigned() const;


    commsdsl::IntField intFieldDslObj() const
    {
        return commsdsl::IntField(dslObj());
    }

private:
    SpecialsList m_specials;
};

inline
FieldPtr createIntField(Generator& generator, commsdsl::Field field)
{
    return std::make_unique<IntField>(generator, field);
}

} // namespace commsdsl2comms
