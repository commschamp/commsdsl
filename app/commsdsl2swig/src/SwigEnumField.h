//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/EnumField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigEnumField final : public commsdsl::gen::EnumField, public SwigField
{
    using Base = commsdsl::gen::EnumField;
    using SwigBase = SwigField;
public:
    SwigEnumField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

    StringsList swigEnumValues() const;

protected:
    // Base overrides
    virtual bool writeImpl() const override;    

    // SwigBase overrides
    virtual std::string swigValueTypeImpl() const override;
    virtual std::string swigExtraPublicFuncsImpl() const override;

private:
    bool swigIsDirectValueNameMappingInternal() const;
};

} // namespace commsdsl2swig
