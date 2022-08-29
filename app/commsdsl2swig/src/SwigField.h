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

#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/Endian.h"

#include <string>
#include <vector>

namespace commsdsl2swig
{

class SwigField
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;
    using SwigFieldsList = std::vector<SwigField*>;

    explicit SwigField(commsdsl::gen::Field& field);
    virtual ~SwigField();

    static SwigFieldsList swigTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields);

    bool swigIsVersionOptional() const;

    // bool swigPrepare();
    bool swigWrite() const;

protected:
    virtual std::string swigMembersDefImpl() const;
    virtual std::string swigValueTypeImpl() const;
    virtual std::string swigValueAccImpl() const;
    virtual std::string swigExtraPublicFuncsImpl() const;

    static const std::string& swigCommonPublicFuncs();
    
private:
    std::string swigClassDefInternal() const;
    std::string swigOptionalDefInternal() const;

    commsdsl::gen::Field& m_field;
};

} // namespace commsdsl2swig
