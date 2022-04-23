//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/EnumField.h"
#include "commsdsl/parse/Protocol.h"

#include "Field.h"
#include "common.h"

namespace commsdsl2old
{

class EnumField final : public Field
{
    using Base = Field;
public:
    EnumField(Generator& generator, commsdsl::parse::Field field) : Base(generator, field) {}

    common::StringsList getValuesList() const;
    std::string getValuesDefinition() const;
    std::string getValueName(std::intmax_t value) const;
    const std::string& underlyingType() const;
    bool isUnsignedUnderlyingType() const;

    unsigned hexWidth() const;

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual void updateIncludesCommonImpl(IncludesList& includes) const override;
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
    virtual std::string getPluginPropertiesImpl(bool serHiddenParam) const override;
    virtual std::string getCommonDefinitionImpl(const std::string& fullScope) const override;

private:
    using StringsList = common::StringsList;

    struct RangeInfo
    {
        std::intmax_t m_min = 0;
        std::intmax_t m_max = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = commsdsl::parse::Protocol::notYetDeprecated();
    };

    using ValidRangesList = std::vector<RangeInfo>;

    std::string getEnumeration(const std::string& scope, bool checkIfMemberChild = true) const;
    std::string getCommonEnumeration(const std::string& fullScope) const;
    std::string getCommonValueNameMapDef() const;
    std::string getFieldBaseParams() const;
    std::string getEnumType(bool isCommon = false) const;
    std::string getFieldOpts(const std::string& scope) const;
    std::string getValid() const;
    std::string getValueNameFunc(bool isCommon = false) const;
    std::string getValueNameWrapFunc(const std::string& scope) const;
    std::string getValueNameFuncDirectBody() const;
    std::string getValueNameFuncBinSearchBody(bool isCommon) const;
    std::string getValueNameBinSearchPairs(bool isCommon) const;
    std::string getBigUnsignedValueNameBinSearchPairs(bool isCommon) const;
    std::string getValueNamesMapFunc(bool isCommon = false) const;
    std::string getValueNamesMapFuncDirectBody() const;
    std::string getValueNamesMapFuncBinSearchBody(bool isCommon) const;
    std::string getValueNamesMapDefs(const std::string& scope) const;
    std::string getValueNamesMapInfoCommonWrapFunc(const std::string& scope) const;
    bool isDirectValueNameMapping() const;
    void checkDefaultValueOpt(StringsList& list) const;
    void checkLengthOpt(StringsList& list) const;
    void checkValidRangesOpt(StringsList& list) const;
    bool prepareRanges() const;
    void updateIncludesForCommonInternal(IncludesList& includes) const;

    commsdsl::parse::EnumField enumFieldDslObj() const
    {
        return commsdsl::parse::EnumField(dslObj());
    }

    mutable ValidRangesList m_validRanges;
};

inline
FieldPtr createEnumField(Generator& generator, commsdsl::parse::Field field)
{
    return std::make_unique<EnumField>(generator, field);
}

} // namespace commsdsl2old