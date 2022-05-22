//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/Endian.h"

#include <string>
#include <vector>

namespace commsdsl2comms
{

class CommsField
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;
    using CommsFieldsList = std::vector<CommsField*>;
    using FieldOptsFunc = std::string (CommsField::*)() const;

    explicit CommsField(commsdsl::gen::Field& field);
    virtual ~CommsField();

    static CommsFieldsList commsTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields);

    bool commsPrepare();
    bool commsWrite() const;

    IncludesList commsCommonIncludes() const;
    std::string commsCommonCode() const;
    bool commsHasMembersCode() const;
    bool commsHasGeneratedReadCode() const;
    bool commsIsVersionDependent() const;
    std::size_t commsMinLength() const;
    std::size_t commsMaxLength() const;

    IncludesList commsDefIncludes() const;
    std::string commsDefCode() const;
    std::string commsDefBundledReadPrepareFuncBody(const CommsFieldsList& siblings) const;
    std::string commsDefBundledRefreshFuncBody(const CommsFieldsList& siblings) const;
    std::string commsCompareToValueCode(
        const std::string& op,
        const std::string& value,
        const std::string& nameOverride = std::string(),
        bool forcedVersionOptional = false) const;

    std::string commsCompareToFieldCode(
        const std::string& op,
        const CommsField& field,
        const std::string& nameOverride = std::string(),
        bool forcedVersionOptional = false) const;                

    bool commsIsVersionOptional() const;

    void commsSetForcedFailOnInvalid()
    {
        m_forcedFailOnInvalid = true;
    }

    void commsSetForcePseudo()
    {
        m_forcedPseudo = true;
    }

    void commsSetReferenced()
    {
        m_referenced = true;
    }

    const commsdsl::gen::Field& field() const
    {
        return m_field;
    }

    std::string commsDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;

    bool commsHasCustomValue() const;

protected:
    virtual IncludesList commsCommonIncludesImpl() const;
    virtual std::string commsCommonCodeBaseClassImpl() const;
    virtual std::string commsCommonCodeBodyImpl() const;
    virtual std::string commsCommonCodeExtraImpl() const;
    virtual std::string commsCommonMembersBaseClassImpl() const;
    virtual std::string commsCommonMembersCodeImpl() const;
    virtual IncludesList commsDefIncludesImpl() const;
    virtual std::string commsDefMembersCodeImpl() const;
    virtual std::string commsDefDoxigenDetailsImpl() const;
    virtual std::string commsDefExtraDoxigenImpl() const;
    virtual std::string commsDefBaseClassImpl() const;
    virtual std::string commsDefPublicCodeImpl() const;
    virtual std::string commsDefProtectedCodeImpl() const;
    virtual std::string commsDefPrivateCodeImpl() const;
    virtual std::string commsDefReadFuncBodyImpl() const;
    virtual StringsList commsDefReadMsvcSuppressWarningsImpl() const;
    virtual std::string commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const;
    virtual std::string commsDefWriteFuncBodyImpl() const;
    virtual std::string commsDefRefreshFuncBodyImpl() const;
    virtual std::string commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const;
    virtual std::string commsDefLengthFuncBodyImpl() const;
    virtual std::string commsDefValidFuncBodyImpl() const;
    virtual bool commsIsLimitedCustomizableImpl() const;
    virtual bool commsIsVersionDependentImpl() const;
    virtual bool commsDefHasNameFuncImpl() const;
    virtual std::string commsCompareToValueCodeImpl(const std::string& op, const std::string& value, const std::string& nameOverride, bool forcedVersionOptional) const;  
    virtual std::string commsCompareToFieldCodeImpl(const std::string& op, const CommsField& field, const std::string& nameOverride, bool forcedVersionOptional) const;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(FieldOptsFunc fieldOptsFunc) const;
    virtual StringsList commsExtraDataViewDefaultOptionsImpl() const;
    virtual StringsList commsExtraBareMetalDefaultOptionsImpl() const;
    virtual std::size_t commsMinLengthImpl() const;
    virtual std::size_t commsMaxLengthImpl() const;    

    std::string commsCommonNameFuncCode() const;
    std::string commsFieldBaseParams(commsdsl::parse::Endian endian) const;
    void commsAddFieldDefOptions(commsdsl::gen::util::StringsList& opts) const;
    bool commsIsFieldCustomizable() const;
    bool commsIsExtended() const;

private:
    using ExtraFieldOptsFunc = StringsList (CommsField::*)() const;

    bool commsPrepareOverrideInternal(
        commsdsl::parse::OverrideType type, 
        std::string& codePathPrefix, 
        const std::string& suffix,
        std::string& customCode,
        const std::string& name);
    bool commsWriteCommonInternal() const;
    bool commsWriteDefInternal() const;
    std::string commsFieldDefCodeInternal() const;
    std::string commsOptionalDefCodeInternal() const;
    std::string commsFieldBriefInternal() const;
    std::string commsDocDetailsInternal() const;
    std::string commsExtraDocInternal() const;
    std::string commsDeprecatedDocInternal() const;
    std::string commsTemplateParamsInternal() const;
    std::string commsDefPublicCodeInternal() const;
    std::string commsDefProtectedCodeInternal() const;
    std::string commsDefPrivateCodeInternal() const;
    std::string commsDefNameFuncCodeInternal() const;
    const std::string& commsDefValueCodeInternal() const;
    std::string commsDefReadFuncCodeInternal() const;
    std::string commsDefWriteFuncCodeInternal() const;
    std::string commsDefRefreshFuncCodeInternal() const;
    std::string commsDefLengthFuncCodeInternal() const;
    std::string commsDefValidFuncCodeInternal() const;
    std::string commsDefMembersCodeInternal() const;
    std::string commsCommonMembersCodeInternal() const;
    std::string commsCustomizationOptionsInternal(
        FieldOptsFunc fieldOptsFunc, 
        ExtraFieldOptsFunc extraFieldOptsFunc,
        bool hasBase) const;
    StringsList commsExtraDataViewDefaultOptionsInternal() const;
    StringsList commsExtraBareMetalDefaultOptionsInternal() const;

    commsdsl::gen::Field& m_field;
    std::string m_customValue;
    std::string m_customRead;
    std::string m_customWrite;
    std::string m_customRefresh;
    std::string m_customLength;
    std::string m_customValid;
    std::string m_customName;
    std::string m_customExtend;
    bool m_forcedFailOnInvalid = false;
    bool m_forcedPseudo = false;
    bool m_referenced = false;
};

} // namespace commsdsl2comms
