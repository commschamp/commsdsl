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

#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2c
{

class CGenerator;
class CMessage final: public commsdsl::gen::GenMessage
{
    using GenBase = commsdsl::gen::GenMessage;

public:
    using ParseMessage = commsdsl::parse::ParseMessage;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit CMessage(CGenerator& generator, ParseMessage parseObj, GenElem* parent);
    virtual ~CMessage();

    [[nodiscard]] static const CMessage* cCast(const commsdsl::gen::GenMessage* i)
    {
        return static_cast<const CMessage*>(i);
    }

    [[nodiscard]] std::string cRelHeader() const;
    [[nodiscard]] std::string cRelCommsHeader() const;
    void cAddSourceFiles(GenStringsList& sources) const;
    [[nodiscard]] std::string cCommsType(bool appendOptions = true) const;
    [[nodiscard]] std::string cName() const;
    [[nodiscard]] std::string cCommsTypeName() const;

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
    [[nodiscard]] std::string cCommsHeaderIncludesInternal() const;
    [[nodiscard]] std::string cCommsHeaderFieldsInternal() const;
    [[nodiscard]] std::string cCommsHeaderCodeInternal() const;

    CFieldsList m_cFields;
};

} // namespace commsdsl2c
