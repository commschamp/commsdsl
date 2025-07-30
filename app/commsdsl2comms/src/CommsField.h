//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/ParseEndian.h"

#include <string>
#include <vector>

namespace commsdsl2comms
{

class CommsField
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenField = commsdsl::gen::GenField;
    using GenFieldsList = GenField::GenFieldsList;

    using CommsIncludesList = GenStringsList;
    using CommsFieldsList = std::vector<CommsField*>;
    using CommsFieldOptsFunc = std::string (CommsField::*)() const;

    explicit CommsField(GenField& field);
    virtual ~CommsField();

    static CommsFieldsList commsTransformFieldsList(const GenFieldsList& fields);

    bool commsPrepare();
    bool commsWrite() const;

    CommsIncludesList commsCommonIncludes() const;
    std::string commsCommonCode() const;
    bool commsHasMembersCode() const;
    bool commsHasGeneratedReadCode() const;
    bool commsIsVersionDependent() const;
    std::size_t commsMinLength() const;
    std::size_t commsMaxLength() const;

    CommsIncludesList commsDefIncludes() const;
    std::string commsDefCode() const;
    std::string commsDefBundledReadPrepareFuncBody(const CommsFieldsList& siblings) const;
    std::string commsDefBundledRefreshFuncBody(const CommsFieldsList& siblings) const;

    std::string commsFieldAccessStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    std::string commsValueAccessStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    std::string commsSizeAccessStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    std::string commsExistsCheckStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    GenStringsList commsCompOptChecks(const std::string& accStr, const std::string& prefix = std::string()) const;
    void commsCompOptChecks(const std::string& accStr, GenStringsList& checks, const std::string& prefix = std::string()) const;
    std::string commsCompValueCastType(const std::string& accStr, const std::string& prefix = std::string()) const;
    std::string commsCompPrepValueStr(const std::string& accStr, const std::string& value) const;
    bool commsVerifyInnerRef(const std::string refStr) const;

    bool commsIsVersionOptional() const;

    void commsSetForcedFailOnInvalid()
    {
        m_forcedFailOnInvalid = true;
    }

    void commsSetForcePseudo()
    {
        m_forcedPseudo = true;
    }

    const GenField& commsGenField() const
    {
        return m_genField;
    }


    std::string commsDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;

    bool commsHasCustomValue() const;
    bool commsHasCustomValid() const;
    bool commsHasCustomLength(bool deepCheck = true) const;
    const CommsField* commsFindSibling(const std::string& name) const;

    bool commsIsFieldCustomizable() const;

protected:
    virtual CommsIncludesList commsCommonIncludesImpl() const;
    virtual std::string commsCommonCodeBaseClassImpl() const;
    virtual std::string commsCommonCodeBodyImpl() const;
    virtual std::string commsCommonCodeExtraImpl() const;
    virtual std::string commsCommonMembersBaseClassImpl() const;
    virtual std::string commsCommonMembersCodeImpl() const;
    virtual CommsIncludesList commsDefIncludesImpl() const;
    virtual std::string commsDefMembersCodeImpl() const;
    virtual std::string commsDefDoxigenDetailsImpl() const;
    virtual std::string commsDefExtraDoxigenImpl() const;
    virtual std::string commsDefBaseClassImpl() const;
    virtual std::string commsDefConstructCodeImpl() const;
    virtual std::string commsDefDestructCodeImpl() const;
    virtual std::string commsDefPublicCodeImpl() const;
    virtual std::string commsDefProtectedCodeImpl() const;
    virtual std::string commsDefPrivateCodeImpl() const;
    virtual std::string commsDefReadFuncBodyImpl() const;
    virtual GenStringsList commsDefReadMsvcSuppressWarningsImpl() const;
    virtual std::string commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const;
    virtual std::string commsDefWriteFuncBodyImpl() const;
    virtual std::string commsDefRefreshFuncBodyImpl() const;
    virtual std::string commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const;
    virtual std::string commsDefLengthFuncBodyImpl() const;
    virtual std::string commsDefValidFuncBodyImpl() const;
    virtual bool commsIsLimitedCustomizableImpl() const;
    virtual bool commsIsVersionDependentImpl() const;
    virtual bool commsDefHasNameFuncImpl() const;
    virtual std::string commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const;
    virtual GenStringsList commsExtraDataViewDefaultOptionsImpl() const;
    virtual GenStringsList commsExtraBareMetalDefaultOptionsImpl() const;
    virtual std::size_t commsMinLengthImpl() const;
    virtual std::size_t commsMaxLengthImpl() const;    
    virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const;
    virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const;
    virtual void commsCompOptChecksImpl(const std::string& accStr, GenStringsList& checks, const std::string& prefix) const;
    virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const;
    virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const;
    virtual bool commsHasCustomLengthDeepImpl() const;
    virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const;
    virtual bool commsMustDefineDefaultConstructorImpl() const;

    std::string commsCommonNameFuncCode() const;
    std::string commsFieldBaseParams(commsdsl::parse::ParseEndian endian) const;
    void commsAddFieldDefOptions(commsdsl::gen::util::GenStringsList& opts, bool tempFieldObj = false) const;
    void commsAddFieldTypeOption(commsdsl::gen::util::GenStringsList& opts) const;
    bool commsIsExtended() const;

private:
    using CommsExtraFieldOptsFunc = GenStringsList (CommsField::*)() const;

    struct CommsCustomCode
    {
        std::string m_value;
        std::string m_read;
        std::string m_write;
        std::string m_refresh;
        std::string m_length;
        std::string m_valid;
        std::string m_name;
        std::string m_inc;
        std::string m_public;
        std::string m_protected;
        std::string m_private;
        std::string m_extend;
        std::string m_append;
    };

    using CommsBodyCustomCodeFunc = std::string (*)(const std::string& codePathPrefix);

    bool commsCopyCodeFromInternal();
    bool commsPrepareOverrideInternal(
        commsdsl::parse::ParseOverrideType type, 
        std::string& codePathPrefix, 
        const std::string& suffix,
        std::string& customCode,
        const std::string& name,
        CommsBodyCustomCodeFunc bodyFunc = nullptr);
    static std::string commsPrepareCustomReadFromBodyInternal(const std::string& codePathPrefix);
    static std::string commsPrepareCustomWriteFromBodyInternal(const std::string& codePathPrefix);
    static std::string commsPrepareCustomRefreshFromBodyInternal(const std::string& codePathPrefix);
    static std::string commsPrepareCustomLengthFromBodyInternal(const std::string& codePathPrefix);
    static std::string commsPrepareCustomValidFromBodyInternal(const std::string& codePathPrefix);
    static std::string commsPrepareCustomNameFromBodyInternal(const std::string& codePathPrefix);
    bool commsWriteCommonInternal() const;
    bool commsWriteDefInternal() const;
    std::string commsFieldDefCodeInternal() const;
    std::string commsOptionalDefCodeInternal() const;
    std::string commsFieldBriefInternal() const;
    std::string commsDocDetailsInternal() const;
    std::string commsExtraDocInternal() const;
    std::string commsDeprecatedDocInternal() const;
    std::string commsTemplateParamsInternal() const;
    std::string commsDefConstructPublicCodeInternal() const;
    std::string commsDefConstructPrivateCodeInternal() const;
    std::string commsDefDestructCodeInternal() const;
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
        CommsFieldOptsFunc fieldOptsFunc, 
        CommsExtraFieldOptsFunc extraFieldOptsFunc,
        bool hasBase) const;
    GenStringsList commsExtraDataViewDefaultOptionsInternal() const;
    GenStringsList commsExtraBareMetalDefaultOptionsInternal() const;

    GenField& m_genField;
    CommsCustomCode m_customCode;
    std::string m_customConstruct;
    bool m_forcedFailOnInvalid = false;
    bool m_forcedPseudo = false;
};

} // namespace commsdsl2comms
