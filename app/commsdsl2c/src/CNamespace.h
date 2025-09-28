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
#include "CMsgId.h"

#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2c
{

class CGenerator;
class CInterface;
class CNamespace final: public commsdsl::gen::GenNamespace
{
    using GenBase = commsdsl::gen::GenNamespace;

public:
    using ParseNamespace = commsdsl::parse::ParseNamespace;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit CNamespace(CGenerator& generator, ParseNamespace parseObj, GenElem* parent);
    virtual ~CNamespace();

    static const CNamespace* cCast(const commsdsl::gen::GenNamespace* schema)
    {
        return static_cast<const CNamespace*>(schema);
    }

    const CInterface* cInterface() const;
    bool cIsSuitableInterface(const CInterface& iFace) const;
    void cAddSourceFiles(GenStringsList& sources) const;
    bool cCodeGenerationAllowed() const;
    std::string cPrefixName() const;

    const CMsgId* cMsgId() const;

protected:
    virtual bool genWriteImpl() const override;

private:
    const CInterface* cFindSuitableInterfaceInternal() const;

    CMsgId m_msgId;
};

} // namespace commsdsl2c
