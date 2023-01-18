//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Interface.h"

#include "CommsField.h"

#include <vector>
#include <string>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsInterface final: public commsdsl::gen::Interface
{
    using Base = commsdsl::gen::Interface;
public:
    using CommsFieldsList = CommsField::CommsFieldsList;

    explicit CommsInterface(CommsGenerator& generator, commsdsl::parse::Interface dslObj, Elem* parent);
    virtual ~CommsInterface();

    const CommsFieldsList& commsFields() const
    {
        return m_commsFields;
    }    

protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;

private:

    bool commsWriteCommonInternal() const;  
    bool commsWriteDefInternal() const;  
    std::string commsCommonIncludesInternal() const;
    std::string commsCommonFieldsCodeInternal() const;
    std::string commsDefIncludesInternal() const;
    std::string commsDefFieldsCodeInternal() const;
    std::string commsDefDocDetailsInternal() const;
    std::string commsDefBaseClassInternal() const;
    std::string commsDefExtraOptionsInternal() const;
    std::string commsDefPublicInternal() const;
    std::string commsDefProtectedInternal() const;
    std::string commsDefPrivateInternal() const;
    std::string commsDefFieldsAccessInternal() const;
    std::string commsDefFieldsAliasesInternal() const;

    std::string m_name;
    std::string m_publicCode;
    std::string m_protectedCode;
    std::string m_privateCode;
    CommsFieldsList m_commsFields;  
};

} // namespace commsdsl2comms
