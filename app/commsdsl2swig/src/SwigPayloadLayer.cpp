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

#include "SwigPayloadLayer.h"

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

SwigPayloadLayer::SwigPayloadLayer(SwigGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

std::string SwigPayloadLayer::swigMemberFieldDeclImpl() const
{
    static const std::string Templ = 
        "class #^#FIELD_TYPE#$#\n"
        "{\n"
        "public:\n"
        "    using ValueType = std::vector<#^#UINT8_T#$#>;\n\n"
        "    const ValueType& getValue() const;\n"
        "    void setValue(const ValueType& val);\n"
        "};\n";

    auto& gen = SwigGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"FIELD_TYPE", swigFieldTypeImpl()},
        {"UINT8_T", gen.swigConvertCppType("std::uint8_t")}
    };

    return util::processTemplate(Templ, repl);
}

void SwigPayloadLayer::swigAddCodeImpl(StringsList& list) const
{
    static const std::string Templ = 
        "class #^#FIELD_TYPE#$# : public #^#COMMS_SCOPE#$#::Field {};\n";

    util::ReplacementMap repl = {
        {"FIELD_TYPE", swigFieldTypeImpl()},
        {"COMMS_SCOPE", swigTemplateScope()}
    };

    list.push_back(util::processTemplate(Templ, repl));
}

std::string SwigPayloadLayer::swigFieldTypeImpl() const
{
    auto& gen = SwigGenerator::cast(generator());
    return gen.swigClassName(*this) + "Field";
}

} // namespace commsdsl2swig
