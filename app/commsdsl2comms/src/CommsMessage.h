//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CommsField.h"

#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsMessage final: public commsdsl::gen::GenMessage
{
    using GenBase = commsdsl::gen::GenMessage;
public:
    using ParseMessage = commsdsl::parse::ParseMessage;

    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    using CommsFieldsList = CommsField::CommsFieldsList;

    explicit CommsMessage(CommsGenerator& generator, ParseMessage parseObj, GenElem* parent);
    virtual ~CommsMessage();

    const CommsFieldsList& commsFields() const
    {
        return m_commsFields;
    }

    std::string commsDefaultOptions() const;
    std::string commsClientDefaultOptions() const;
    std::string commsServerDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    using CommsFieldOptsFunc = std::string (CommsField::*)() const;
    using CommsExtraMessageOptsFunc = GenStringsList (CommsMessage::*)() const;

    struct CommsCustomCode
    {
        std::string m_constructBody;
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

    using CommsCustomCodeFunc = std::string (CommsMessage::*)(bool& hasCode) const;

    bool commsCopyCodeFromInternal();
    bool commsPrepareOverrideInternal(
        commsdsl::parse::ParseOverrideType type,
        const std::string& name,
        CommsCustomCodeFunc codeFunc,
        std::string& code,
        bool* hasCode);

    std::string commsCustomReadCodeInternal(bool& hasRealCode) const;
    std::string commsCustomWriteCodeInternal(bool& hasRealCode) const;
    std::string commsCustomRefreshCodeInternal(bool& hasRealCode) const;
    std::string commsCustomLengthCodeInternal(bool& hasRealCode) const;
    std::string commsCustomValidCodeInternal(bool& hasRealCode) const;
    std::string commsCustomNameCodeInternal(bool& hasRealCode) const;
    bool commsWriteCommonInternal() const;
    bool commsWriteDefInternal() const;
    std::string commsCommonIncludesInternal() const;
    std::string commsCommonBodyInternal() const;
    std::string commsCommonNameFuncInternal() const;
    std::string commsCommonFieldsCodeInternal() const;
    std::string commsDefIncludesInternal() const;
    std::string commsDefConstructInternal() const;
    std::string commsDefFieldsCodeInternal() const;
    std::string commsDefFieldClassNamesListInternal() const;
    std::string commsDefDocDetailsInternal() const;
    std::string commsDefDeprecatedDocInternal() const;
    std::string commsDefBaseClassInternal() const;
    std::string commsDefCustomizationOptInternal() const;
    std::string commsDefExtraOptionsInternal() const;
    std::string commsDefPublicInternal() const;
    std::string commsDefProtectedInternal() const;
    std::string commsDefPrivateInternal() const;
    std::string commsDefFieldsAccessInternal() const;
    std::string commsDefFieldsAliasesInternal() const;
    std::string commsDefLengthCheckInternal() const;
    std::string commsDefNameFuncInternal() const;
    std::string commsDefReadFuncInternal() const;
    std::string commsDefRefreshFuncInternal() const;
    std::string commsDefPrivateConstructInternal() const;
    bool commsIsCustomizableInternal() const;
    std::string commsCustomizationOptionsInternal(
        CommsFieldOptsFunc fieldOptsFunc,
        CommsExtraMessageOptsFunc extraMessageOptsFunc,
        bool hasBase) const;
    std::string commsDefReadConditionsCodeInternal() const;
    std::string commsDefOrigValidCodeInternal() const;
    std::string commsDefValidFuncInternal() const;

    GenStringsList commsClientExtraCustomizationOptionsInternal() const;
    GenStringsList commsServerExtraCustomizationOptionsInternal() const;
    void commsPrepareConstructCodeInternal();

    CommsFieldsList m_commsFields;
    commsdsl::gen::util::GenStringsList m_bundledReadPrepareCodes;
    commsdsl::gen::util::GenStringsList m_bundledRefreshCodes;
    std::string m_internalConstruct;
    CommsCustomCode m_customCode;
    CommsExtraCustomCode m_extraCustomCode;
};

} // namespace commsdsl2comms
