//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenListField.h"

namespace commsdsl2c
{

class CGenerator;
class CListField final : public commsdsl::gen::GenListField, public CField
{
    using GenBase = commsdsl::gen::GenListField;
    using CBase = CField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    CListField(CGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    // GenBase overrides
    virtual bool genWriteImpl() const override;    

    // CBase overrides

private:
};

} // namespace commsdsl2c
