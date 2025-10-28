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
    bool cWrite() const;

    std::string cRelHeader() const;
    std::string cRelCommsHeader() const;
    std::string cRelSource() const;
    std::string cName() const;
    std::string cCommsTypeName() const;
    void cAddSourceFiles(GenStringsList& sources) const;

private:
    bool cWriteHeaderInternal() const;
    bool cWriteSourceInternal() const;
    bool cWriteCommsHeaderInternal() const;
    std::string cHeaderFuncsInternal() const;
    std::string cHeaderFwdInternal() const;
    std::string cCommsHeaderIncludesInternal() const;
    std::string cCommsHeaderFuncsInternal() const;
    std::string cCommsSourceIncludesInternal() const;
    std::string cCommsSourceFuncsInternal() const;
    GenMessagesAccessList cMessagesListInternal() const;

    CGenerator& m_cGenerator;
    const CNamespace& m_parent;
};

} // namespace commsdsl2c