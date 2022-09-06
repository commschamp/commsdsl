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

#include "SwigBitfieldField.h"

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

SwigBitfieldField::SwigBitfieldField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigBitfieldField::prepareImpl()
{
    return 
        Base::prepareImpl() &&
        swigPrepareInternal();
}

bool SwigBitfieldField::writeImpl() const
{
    return swigWrite();
}

bool SwigBitfieldField::swigPrepareInternal()
{
    m_swigMembers = swigTransformFieldsList(members());
    return true;
}

std::string SwigBitfieldField::swigMembersDeclImpl() const
{
    StringsList memberDefs;
    memberDefs.reserve(m_swigMembers.size());

    for (auto* m : m_swigMembers) {
        memberDefs.push_back(m->swigClassDecl());
    }

    return util::strListToString(memberDefs, "\n", "\n");
}

std::string SwigBitfieldField::swigValueAccDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigBitfieldField::swigExtraPublicFuncsDeclImpl() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigMembers.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* m : m_swigMembers) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#& field_#^#ACC_NAME#$#();\n"
        ;

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(m->field())},
            {"ACC_NAME", comms::accessName(m->field().dslObj().name())}
        };

        accFuncs.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(accFuncs, "\n", "");
}

void SwigBitfieldField::swigAddDefImpl(StringsList& list) const
{
    for (auto* m : m_swigMembers) {
        m->swigAddDef(list);
    }    
}

} // namespace commsdsl2swig
