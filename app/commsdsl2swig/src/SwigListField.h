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

#include "commsdsl/gen/GenListField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigListField final : public commsdsl::gen::GenListField, public SwigField
{
    using GenBase = commsdsl::gen::GenListField;
    using SwigBase = SwigField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    SwigListField(SwigGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    // GenBase overrides
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

    // SwigBase overrides
    virtual std::string swigMembersDeclImpl() const override;
    virtual std::string swigValueTypeDeclImpl() const override;
    virtual std::string swigValueAccDeclImpl() const override;
    virtual std::string swigExtraPublicFuncsCodeImpl() const override;
    virtual void swigAddDefImpl(GenStringsList& list) const override;
    virtual void swigAddMembersCodeImpl(GenStringsList& list) const override;
};

} // namespace commsdsl2swig
