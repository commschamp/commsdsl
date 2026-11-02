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

#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2c
{

class CGenerator;
class CNamespace;

class CMsgHandler
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenMessagesAccessList = commsdsl::gen::GenNamespace::GenMessagesAccessList;

    CMsgHandler(CGenerator& generator, const CNamespace& parent);
    [[nodiscard]] bool cWrite() const;

    [[nodiscard]] std::string cRelHeader() const;
    [[nodiscard]] std::string cRelCommsHeader() const;
    [[nodiscard]] std::string cRelSource() const;
    [[nodiscard]] std::string cName() const;
    [[nodiscard]] std::string cCommsTypeName() const;
    void cAddSourceFiles(GenStringsList& sources) const;

private:
    [[nodiscard]] bool cWriteHeaderInternal() const;
    [[nodiscard]] bool cWriteSourceInternal() const;
    [[nodiscard]] bool cWriteCommsHeaderInternal() const;
    [[nodiscard]] std::string cHeaderFuncsInternal() const;
    [[nodiscard]] std::string cHeaderFwdInternal() const;
    [[nodiscard]] std::string cCommsHeaderIncludesInternal() const;
    [[nodiscard]] std::string cCommsHeaderFuncsInternal() const;
    [[nodiscard]] std::string cCommsSourceIncludesInternal() const;
    [[nodiscard]] std::string cCommsSourceFuncsInternal() const;
    [[nodiscard]] GenMessagesAccessList cMessagesListInternal() const;

    CGenerator& m_cGenerator;
    const CNamespace& m_parent;
};

} // namespace commsdsl2c