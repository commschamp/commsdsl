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

#include "SwigDataField.h"

#include "SwigGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

SwigDataField::SwigDataField(SwigGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigDataField::genWriteImpl() const
{
    return swigWrite();
}

std::string SwigDataField::swigValueTypeDeclImpl() const
{
    static const std::string Templ = 
        "using ValueType = std::vector<#^#UINT8_T#$#>;\n";

    auto& gen = SwigGenerator::cast(genGenerator());
    util::GenReplacementMap repl = {
        {"UINT8_T", gen.swigConvertCppType("std::uint8_t")}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigDataField::swigValueAccDeclImpl() const
{
    return 
        "ValueType& value();\n" + 
        SwigBase::swigValueAccDeclImpl();
}


} // namespace commsdsl2swig
