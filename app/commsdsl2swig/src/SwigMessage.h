//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "SwigField.h"

#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigInterface;
class SwigMsgHandler;
class SwigMessage final: public commsdsl::gen::GenMessage
{
    using GenBase = commsdsl::gen::GenMessage;

public:
    using ParseMessage = commsdsl::parse::ParseMessage;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit SwigMessage(SwigGenerator& generator, ParseMessage parseObj, GenElem* parent);
    virtual ~SwigMessage();

    void swigAddCodeIncludes(GenStringsList& list) const;
    void swigAddCode(GenStringsList& list) const;
    void swigAddDef(GenStringsList& list) const;

    [[nodiscard]] static const SwigMessage* swigCast(const commsdsl::gen::GenMessage* obj)
    {
        return static_cast<const SwigMessage*>(obj);
    }

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

private:
    using SwigFieldsList = SwigField::SwigFieldsList;

    [[nodiscard]] std::string swigFieldDefsInternal() const;
    [[nodiscard]] std::string swigClassDeclInternal() const;
    [[nodiscard]] std::string swigFieldsAccDeclInternal() const;
    [[nodiscard]] std::string swigFieldsAccCodeInternal() const;
    [[nodiscard]] const SwigInterface* swigGetInterfaceInternal() const;
    [[nodiscard]] const SwigMsgHandler* swigMsgHandlerInternal() const;

    SwigFieldsList m_swigFields;
};

} // namespace commsdsl2swig
