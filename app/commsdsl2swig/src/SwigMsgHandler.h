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

#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2swig
{

class SwigGenerator;
class SwigNamespace;

class SwigMsgHandler
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenMessagesAccessList = commsdsl::gen::GenNamespace::GenMessagesAccessList;

    SwigMsgHandler(SwigGenerator& generator, const SwigNamespace& parent);
    bool swigWrite() const;

    void swigAddFwdCode(GenStringsList& list) const;
    void swigAddClassCode(GenStringsList& list) const;
    void swigAddDef(GenStringsList& list) const;
    std::string swigClassName() const;

private:
    std::string swigClassDeclInternal() const;
    GenMessagesAccessList swigMessagesListInternal() const;

    SwigGenerator& m_swigGenerator;
    const SwigNamespace& m_parent;

    mutable bool m_written = false;
    mutable bool m_fwdAdded = false;
    mutable bool m_codeAdded = false;
    mutable bool m_defAdded = false;
};

} // namespace commsdsl2swig