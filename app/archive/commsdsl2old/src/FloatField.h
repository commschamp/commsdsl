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

#include "commsdsl/parse/FloatField.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2old
{

class FloatField final : public Field
{
    using Base = Field;
public:
    FloatField(Generator& generator, commsdsl::parse::Field field) : Base(generator, field) {}

protected:
    virtual bool prepareImpl() override;
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual void updateIncludesCommonImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        const std::string& className) const override;
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override;
    virtual std::string getCommonDefinitionImpl(const std::string& fullScope) const override;

private:
    using StringsList = common::StringsList;
    using SpecialsListElem = std::pair<std::string, commsdsl::parse::FloatField::SpecialValueInfo>;
    using SpecialsList = std::vector<SpecialsListElem>;

    std::string getFieldBaseParams() const;
    const std::string& getFieldType() const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getConstructor() const;
    std::string getSpecialNamesMapDefs(const std::string& scope = std::string()) const;
    static const std::string& getCommonSpecialNameInfoDef();
    static const std::string& getCommonSpecialNamesMapDef();
    std::string getSpecialNameInfoDef(const std::string& scope) const;
    std::string getSpecialNamesMapDef(const std::string& scope) const;   
    std::string getHasSpecialsFunc(const std::string& scope = std::string()) const; 
    std::string getHasSpecialsFuncCommonBody() const;
    std::string getHasSpecialsFuncBody(const std::string& scope) const;
    std::string getSpecials(const std::string& scope) const;
    std::string getSpacialNamesMapFunc(const std::string& scope = std::string()) const;
    std::string getSpacialNamesMapFuncCommonBody() const;
    std::string getSpacialNamesMapFuncBody(const std::string& scope) const;
    std::string getValid() const;
    std::string getDisplayDecimals() const;
    StringsList getVersionBasedConditions() const;
    StringsList getNormalConditions() const;
    void checkUnitsOpt(StringsList& list) const;
    void checkVersionOpt(StringsList& list) const;
    void checkValidityOpt(StringsList& list) const;

    commsdsl::parse::FloatField floatFieldDslObj() const
    {
        return commsdsl::parse::FloatField(dslObj());
    }

private:
    SpecialsList m_specials;
};

inline
FieldPtr createFloatField(Generator& generator, commsdsl::parse::Field field)
{
    return std::make_unique<FloatField>(generator, field);
}

} // namespace commsdsl2old