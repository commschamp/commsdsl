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

#include "CommsField.h"

#include <vector>
#include <string>

namespace commsdsl2new
{

class CommsGenerator;
class CommsMessage final: public commsdsl::gen::Message
{
    using Base = commsdsl::gen::Message;
public:
    explicit CommsMessage(CommsGenerator& generator, commsdsl::parse::Message dslObj, Elem* parent);
    virtual ~CommsMessage();

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() override;

private:
    using CommsFieldsList = std::vector<CommsField*>;
    struct RefreshCodeInfo
    {
        RefreshCodeInfo(std::string&& accName, std::string&& code) :
            m_accName(std::move(accName)),
            m_code(std::move(code))
        {
        }

        std::string m_accName;
        std::string m_code;
    };

    bool commsWriteCommonInternal();
    bool commsWriteDefInternal();  
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

    CommsFieldsList m_commsFields;  
    std::vector<RefreshCodeInfo> m_fieldsRefresh;
    std::string m_customRefresh;
};

} // namespace commsdsl2new
