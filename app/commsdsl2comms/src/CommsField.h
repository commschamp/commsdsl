//
// Copyright 2019 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

    [[nodiscard]] static CommsFieldsList commsTransformFieldsList(const GenFieldsList& fields);
    [[nodiscard]] static const CommsField* commsCast(const GenField* field);

    [[nodiscard]] bool commsPrepare();
    [[nodiscard]] bool commsWrite() const;

    [[nodiscard]] CommsIncludesList commsCommonIncludes() const;
    [[nodiscard]] std::string commsCommonCode() const;
    [[nodiscard]] bool commsHasMembersCode() const;
    [[nodiscard]] bool commsHasGeneratedReadCode() const;
    [[nodiscard]] bool commsIsVersionDependent() const;
    [[nodiscard]] std::size_t commsMinLength() const;
    [[nodiscard]] std::size_t commsMaxLength() const;

    [[nodiscard]] CommsIncludesList commsDefIncludes() const;
    [[nodiscard]] std::string commsDefCode() const;
    [[nodiscard]] std::string commsDefBundledReadPrepareFuncBody(const CommsFieldsList& siblings) const;
    [[nodiscard]] std::string commsDefBundledRefreshFuncBody(const CommsFieldsList& siblings) const;

    [[nodiscard]] std::string commsFieldAccessStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    [[nodiscard]] std::string commsValueAccessStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    [[nodiscard]] std::string commsSizeAccessStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    [[nodiscard]] std::string commsExistsCheckStr(const std::string& accStr, const std::string& prefix = std::string()) const;
    [[nodiscard]] GenStringsList commsCompOptChecks(const std::string& accStr, const std::string& prefix = std::string()) const;
    void commsCompOptChecks(const std::string& accStr, GenStringsList& checks, const std::string& prefix = std::string()) const;
    [[nodiscard]] std::string commsCompValueCastType(const std::string& accStr, const std::string& prefix = std::string()) const;
    [[nodiscard]] std::string commsCompPrepValueStr(const std::string& accStr, const std::string& value) const;
    [[nodiscard]] bool commsVerifyInnerRef(const std::string refStr) const;

    [[nodiscard]] bool commsIsVersionOptional() const;

    void commsSetForcedFailOnInvalid()
    {
        m_forcedFailOnInvalid = true;
    }

    void commsSetForcePseudo()
    {
        m_forcedPseudo = true;
    }

    [[nodiscard]] const GenField& commsGenField() const
    {
        return m_genField;
    }

    [[nodiscard]] std::string commsDefaultOptions() const;
    [[nodiscard]] std::string commsDataViewDefaultOptions() const;
    [[nodiscard]] std::string commsBareMetalDefaultOptions() const;

    [[nodiscard]] bool commsHasCustomValue() const;
    [[nodiscard]] bool commsHasCustomRead() const;
    [[nodiscard]] bool commsHasCustomValid() const;
    [[nodiscard]] bool commsHasCustomLength(bool deepCheck = true) const;
    [[nodiscard]] const CommsField* commsFindSibling(const std::string& name) const;

    [[nodiscard]] bool commsIsFieldCustomizable() const;

    static void commsAppendCustomCode(
        const std::string& code,
        bool hasCode,
        std::string& to,
        bool& toHasCode);
    static void commsAssignCustomCode(
        const std::string& code,
        bool hasCode,
        std::string& to,
        bool& toHasCode);

protected:
    [[nodiscard]] virtual CommsIncludesList commsCommonIncludesImpl() const;
    [[nodiscard]] virtual std::string commsCommonCodeBaseClassImpl() const;
    [[nodiscard]] virtual std::string commsCommonCodeBodyImpl() const;
    [[nodiscard]] virtual std::string commsCommonCodeExtraImpl() const;
    [[nodiscard]] virtual std::string commsCommonMembersBaseClassImpl() const;
    [[nodiscard]] virtual std::string commsCommonMembersCodeImpl() const;
    [[nodiscard]] virtual CommsIncludesList commsDefIncludesImpl() const;
    [[nodiscard]] virtual std::string commsDefMembersCodeImpl() const;
    [[nodiscard]] virtual std::string commsDefDoxigenDetailsImpl() const;
    [[nodiscard]] virtual std::string commsDefExtraDoxigenImpl() const;
    [[nodiscard]] virtual std::string commsDefBaseClassImpl() const;
    [[nodiscard]] virtual std::string commsDefConstructCodeImpl() const;
    [[nodiscard]] virtual std::string commsDefDestructCodeImpl() const;
    [[nodiscard]] virtual std::string commsDefPublicCodeImpl() const;
    [[nodiscard]] virtual std::string commsDefProtectedCodeImpl() const;
    [[nodiscard]] virtual std::string commsDefPrivateCodeImpl() const;
    [[nodiscard]] virtual std::string commsDefReadFuncBodyImpl() const;
    [[nodiscard]] virtual GenStringsList commsDefReadMsvcSuppressWarningsImpl() const;
    [[nodiscard]] virtual std::string commsDefBundledReadPrepareFuncBodyImpl(const CommsFieldsList& siblings) const;
    [[nodiscard]] virtual std::string commsDefWriteFuncBodyImpl() const;
    [[nodiscard]] virtual std::string commsDefRefreshFuncBodyImpl() const;
    [[nodiscard]] virtual std::string commsDefBundledRefreshFuncBodyImpl(const CommsFieldsList& siblings) const;
    [[nodiscard]] virtual std::string commsDefLengthFuncBodyImpl() const;
    [[nodiscard]] virtual std::string commsDefValidFuncBodyImpl() const;
    [[nodiscard]] virtual bool commsIsLimitedCustomizableImpl() const;
    [[nodiscard]] virtual bool commsIsVersionDependentImpl() const;
    [[nodiscard]] virtual bool commsDefHasNameFuncImpl() const;
    [[nodiscard]] virtual std::string commsMembersCustomizationOptionsBodyImpl(CommsFieldOptsFunc fieldOptsFunc) const;
    [[nodiscard]] virtual GenStringsList commsExtraDataViewDefaultOptionsImpl() const;
    [[nodiscard]] virtual GenStringsList commsExtraBareMetalDefaultOptionsImpl() const;
    [[nodiscard]] virtual std::size_t commsMinLengthImpl() const;
    [[nodiscard]] virtual std::size_t commsMaxLengthImpl() const;
    [[nodiscard]] virtual std::string commsValueAccessStrImpl(const std::string& accStr, const std::string& prefix) const;
    [[nodiscard]] virtual std::string commsSizeAccessStrImpl(const std::string& accStr, const std::string& prefix) const;
    virtual void commsCompOptChecksImpl(const std::string& accStr, GenStringsList& checks, const std::string& prefix) const;
    [[nodiscard]] virtual std::string commsCompValueCastTypeImpl(const std::string& accStr, const std::string& prefix) const;
    [[nodiscard]] virtual std::string commsCompPrepValueStrImpl(const std::string& accStr, const std::string& value) const;
    [[nodiscard]] virtual bool commsHasCustomLengthDeepImpl() const;
    [[nodiscard]] virtual bool commsVerifyInnerRefImpl(const std::string& refStr) const;
    [[nodiscard]] virtual bool commsMustDefineDefaultConstructorImpl() const;

    [[nodiscard]] std::string commsCommonNameFuncCode() const;
    [[nodiscard]] std::string commsFieldBaseParams(commsdsl::parse::ParseEndian endian) const;
    void commsAddFieldDefOptions(commsdsl::gen::util::GenStringsList& opts, bool tempFieldObj = false) const;
    void commsAddFieldTypeOption(commsdsl::gen::util::GenStringsList& opts) const;
    [[nodiscard]] bool commsIsExtended() const;

private:
    using CommsExtraFieldOptsFunc = GenStringsList (CommsField::*)() const;
    using CommsCustomCodeFunc = std::string (CommsField::*)(bool& hasCode) const;

    struct CommsCustomCode
    {
        std::string m_constructBody;
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
        std::string m_append;

        bool m_hasConstructBody = false;
        bool m_hasValue = false;
        bool m_hasRead = false;
        bool m_hasWrite = false;
        bool m_hasRefresh = false;
        bool m_hasLength = false;
        bool m_hasValid = false;
        bool m_hasName = false;
        bool m_hasInc = false;
        bool m_hasPublic = false;
        bool m_hasProtected = false;
        bool m_hasPrivate = false;
        bool m_hasAppend = false;
    };

    struct CommsExtraCustomCode
    {
        std::string m_construct;
        std::string m_extend;

        bool m_hasConstruct = false;
        bool m_hasExtend = false;
    };

    using CommsBodyCustomCodeFunc = std::string (*)(const std::string& codePathPrefix);

    [[nodiscard]] bool commsCopyCodeFromInternal();
    [[nodiscard]] bool commsPrepareOverrideInternal(
        commsdsl::parse::ParseOverrideType type,
        const std::string& name,
        CommsCustomCodeFunc codeFunc,
        std::string& code,
        bool& hasCode);

    [[nodiscard]] std::string commsCustomValueCodeInternal(bool& hasRealCode) const;
    [[nodiscard]] std::string commsCustomReadCodeInternal(bool& hasRealCode) const;
    [[nodiscard]] std::string commsCustomWriteCodeInternal(bool& hasRealCode) const;
    [[nodiscard]] std::string commsCustomRefreshCodeInternal(bool& hasRealCode) const;
    [[nodiscard]] std::string commsCustomLengthCodeInternal(bool& hasRealCode) const;
    [[nodiscard]] std::string commsCustomValidCodeInternal(bool& hasRealCode) const;
    [[nodiscard]] std::string commsCustomNameCodeInternal(bool& hasRealCode) const;
    [[nodiscard]] bool commsWriteCommonInternal() const;
    [[nodiscard]] bool commsWriteDefInternal() const;
    [[nodiscard]] std::string commsFieldDefCodeInternal() const;
    [[nodiscard]] std::string commsOptionalDefCodeInternal() const;
    [[nodiscard]] std::string commsFieldBriefInternal() const;
    [[nodiscard]] std::string commsDocDetailsInternal() const;
    [[nodiscard]] std::string commsExtraDocInternal() const;
    [[nodiscard]] std::string commsDeprecatedDocInternal() const;
    [[nodiscard]] std::string commsTemplateParamsInternal() const;
    [[nodiscard]] std::string commsDefConstructPublicCodeInternal() const;
    [[nodiscard]] std::string commsDefConstructPrivateCodeInternal() const;
    [[nodiscard]] std::string commsDefDestructCodeInternal() const;
    [[nodiscard]] std::string commsDefPublicCodeInternal() const;
    [[nodiscard]] std::string commsDefProtectedCodeInternal() const;
    [[nodiscard]] std::string commsDefPrivateCodeInternal() const;
    [[nodiscard]] std::string commsDefNameFuncCodeInternal() const;
    [[nodiscard]] const std::string& commsDefValueCodeInternal() const;
    [[nodiscard]] std::string commsDefReadFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefWriteFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefRefreshFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefLengthFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefValidFuncCodeInternal() const;
    [[nodiscard]] std::string commsDefMembersCodeInternal() const;
    [[nodiscard]] std::string commsCommonMembersCodeInternal() const;
    [[nodiscard]] std::string commsCustomizationOptionsInternal(
        CommsFieldOptsFunc fieldOptsFunc,
        CommsExtraFieldOptsFunc extraFieldOptsFunc,
        bool hasBase) const;
    [[nodiscard]] GenStringsList commsExtraDataViewDefaultOptionsInternal() const;
    [[nodiscard]] GenStringsList commsExtraBareMetalDefaultOptionsInternal() const;

    GenField& m_genField;
    CommsCustomCode m_customCode;
    CommsExtraCustomCode m_extraCustomCode;
    bool m_forcedFailOnInvalid = false;
    bool m_forcedPseudo = false;
};

} // namespace commsdsl2comms
