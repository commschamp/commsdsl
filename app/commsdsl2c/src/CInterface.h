//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CField.h"

#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2c
{

class CGenerator;
class CMsgId;
class CInterface final: public commsdsl::gen::GenInterface
{
    using GenBase = commsdsl::gen::GenInterface;

public:
    using ParseInterface = commsdsl::parse::ParseInterface;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit CInterface(CGenerator& generator, ParseInterface parseObj, GenElem* parent);
    virtual ~CInterface();

    static const CInterface* cCast(const commsdsl::gen::GenInterface* schema)
    {
        return static_cast<const CInterface*>(schema);
    }

    std::string cRelHeader() const;
    std::string cRelCommsDefHeader() const;
    void cAddSourceFiles(GenStringsList& sources) const;
    std::string cCommsType() const;
    bool cCodeGenerationAllowed() const;
    std::string cName() const;
    std::string cCommsTypeName() const;

    const CMsgId* cMsgId() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    using CFieldsList = CField::CFieldsList;

    bool cWriteHeaderInternal() const;
    bool cWriteSourceInternal() const;
    bool cWriteCommsHeaderInternal() const;

    std::string cHeaderIncludesInternal() const;
    std::string cHeaderFieldsInternal() const;
    std::string cHeaderCodeInternal() const;
    std::string cSourceIncludesInternal() const;
    std::string cSourceFieldsInternal() const;
    std::string cSourceCodeInternal() const;

    CFieldsList m_cFields;
};

} // namespace commsdsl2c
