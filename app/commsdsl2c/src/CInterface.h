//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "CField.h"

#include "commsdsl/gen/GenInterface.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2c
{

class CGenerator;
class CMsgId;
class CMsgHandler;

class CInterface final: public commsdsl::gen::GenInterface
{
    using GenBase = commsdsl::gen::GenInterface;

public:
    using ParseInterface = commsdsl::parse::ParseInterface;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit CInterface(CGenerator& generator, ParseInterface parseObj, GenElem* parent);
    virtual ~CInterface();

    [[nodiscard]] static const CInterface* cCast(const commsdsl::gen::GenInterface* obj)
    {
        return static_cast<const CInterface*>(obj);
    }

    [[nodiscard]] std::string cRelHeader() const;
    [[nodiscard]] std::string cRelCommsHeader() const;
    void cAddSourceFiles(GenStringsList& sources) const;
    [[nodiscard]] std::string cCommsType() const;
    [[nodiscard]] bool cCodeGenerationAllowed() const;
    [[nodiscard]] std::string cName() const;
    [[nodiscard]] std::string cCommsTypeName() const;

    [[nodiscard]] const CMsgId* cMsgId() const;
    [[nodiscard]] const CMsgHandler* cMsgHandler() const;

    [[nodiscard]] bool cIsPresent() const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

private:
    using CFieldsList = CField::CFieldsList;

    [[nodiscard]] bool cWriteHeaderInternal() const;
    [[nodiscard]] bool cWriteSourceInternal() const;
    [[nodiscard]] bool cWriteCommsHeaderInternal() const;

    [[nodiscard]] std::string cHeaderIncludesInternal() const;
    [[nodiscard]] std::string cHeaderFieldsInternal() const;
    [[nodiscard]] std::string cHeaderCodeInternal() const;
    [[nodiscard]] std::string cSourceIncludesInternal() const;
    [[nodiscard]] std::string cSourceFieldsInternal() const;
    [[nodiscard]] std::string cSourceCodeInternal() const;

    CFieldsList m_cFields;
};

} // namespace commsdsl2c
