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

#include "SwigField.h"
#include "SwigMsgId.h"

#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigNamespace final: public commsdsl::gen::GenNamespace
{
    using GenBase = commsdsl::gen::GenNamespace;

public:
    using ParseNamespace = commsdsl::parse::ParseNamespace;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit SwigNamespace(SwigGenerator& generator, ParseNamespace parseObj, GenElem* parent);
    virtual ~SwigNamespace();

    void swigAddCodeIncludes(GenStringsList& list) const;
    void swigAddCode(GenStringsList& list) const;

    void swigAddDef(GenStringsList& list) const;

    static const SwigNamespace* swigCast(const commsdsl::gen::GenNamespace* ns)
    {
        return static_cast<const SwigNamespace*>(ns);
    }

    std::string swigMsgIdClassName() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    using SwigFieldsList = SwigField::SwigFieldsList;

    SwigFieldsList m_swigFields;
    SwigMsgId m_msgId;
};

} // namespace commsdsl2swig
