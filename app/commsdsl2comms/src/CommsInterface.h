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

#include "commsdsl/gen/GenInterface.h"

#include "CommsField.h"

#include <vector>
#include <string>

namespace commsdsl2comms
{

class CommsGenerator;
class CommsInterface final: public commsdsl::gen::GenInterface
{
    using Base = commsdsl::gen::GenInterface;
public:
    using CommsFieldsList = CommsField::CommsFieldsList;

    explicit CommsInterface(CommsGenerator& generator, commsdsl::parse::ParseInterface dslObj, commsdsl::gen::GenElem* parent);
    virtual ~CommsInterface();

    static const CommsInterface* cast(const commsdsl::gen::GenInterface* value)
    {
        return static_cast<const CommsInterface*>(value);
    }

    const CommsFieldsList& commsFields() const
    {
        return m_commsFields;
    }    

    const CommsField* findValidReferencedField(const std::string& refStr) const;


protected:
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;

private:
    struct CustomCode
    {
        std::string m_inc;
        std::string m_public;
        std::string m_protected;
        std::string m_private;
        std::string m_extend;
        std::string m_append;
    };

    bool copyCodeFromInternal();
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
    std::string m_constructCode;
    // std::string m_publicCode;
    // std::string m_protectedCode;
    // std::string m_privateCode;
    CommsFieldsList m_commsFields;  
    CustomCode m_customCode;
};

} // namespace commsdsl2comms
