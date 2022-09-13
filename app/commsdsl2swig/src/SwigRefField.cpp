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

#include "SwigRefField.h"

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

SwigRefField::SwigRefField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigRefField::writeImpl() const
{
    return swigWrite();
}

std::string SwigRefField::swigBaseClassDeclImpl() const
{
    auto* field = SwigField::cast(referencedField());
    assert(field != nullptr);
    return SwigGenerator::cast(generator()).swigClassName(field->field());
}

std::string SwigRefField::swigValueAccDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigRefField::swigCommonPublicFuncsDeclImpl() const
{
    static const std::string Templ = 
        "static const char* name();\n"
    ;

    return Templ;
}

void SwigRefField::swigAddDefImpl(StringsList& list) const
{
    // Make sure the referenced field is defined before
    SwigField::cast(referencedField())->swigAddDef(list);
}

void SwigRefField::swigAddMembersCodeImpl(StringsList& list) const
{
    // Make sure the referenced field is defined before
    SwigField::cast(referencedField())->swigAddCode(list);
}

} // namespace commsdsl2swig
