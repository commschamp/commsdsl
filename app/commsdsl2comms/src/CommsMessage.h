//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Message.h"
#include "commsdsl/gen/util.h"

#include "CommsField.h"

#include <vector>
#include <string>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsMessage final: public commsdsl::gen::Message
{
    using Base = commsdsl::gen::Message;
public:
    using StringsList = commsdsl::gen::util::StringsList;

    explicit CommsMessage(CommsGenerator& generator, commsdsl::parse::Message dslObj, Elem* parent);
    virtual ~CommsMessage();

    std::string commsDefaultOptions() const;
    std::string commsClientDefaultOptions() const;
    std::string commsServerDefaultOptions() const;
    std::string commsDataViewDefaultOptions() const;
    std::string commsBareMetalDefaultOptions() const;

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;

private:
    using CommsFieldsList = CommsField::CommsFieldsList;
    using FieldOptsFunc = std::string (CommsField::*)() const;
    using ExtraMessageOptsFunc = StringsList (CommsMessage::*)() const;

    bool commsPrepareOverrideInternal(
        commsdsl::parse::OverrideType type, 
        std::string& codePathPrefix, 
        const std::string& suffix,
        std::string& customCode,
        const std::string& name);
    bool commsWriteCommonInternal() const;
    bool commsWriteDefInternal() const;  
    std::string commsCommonIncludesInternal() const;
    std::string commsCommonBodyInternal() const;
    std::string commsCommonNameFuncInternal() const;
    std::string commsCommonFieldsCodeInternal() const;
    std::string commsDefIncludesInternal() const;
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
    bool commsIsCustomizableInternal() const;
    // bool commsMustGenerateReadRefresh() const;
    std::string commsCustomizationOptionsInternal(
        FieldOptsFunc fieldOptsFunc,
        ExtraMessageOptsFunc extraMessageOptsFunc,
        bool hasBase) const;

    StringsList commsClientExtraCustomizationOptionsInternal() const;
    StringsList commsServerExtraCustomizationOptionsInternal() const;

    CommsFieldsList m_commsFields;  
    commsdsl::gen::util::StringsList m_bundledReadPrepareCodes;
    commsdsl::gen::util::StringsList m_bundledRefreshCodes;
    std::string m_customRead;
    std::string m_customWrite;
    std::string m_customRefresh;
    std::string m_customLength;
    std::string m_customValid;
    std::string m_customName;
    std::string m_customExtend;
};

} // namespace commsdsl2comms
