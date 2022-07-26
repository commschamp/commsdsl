//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtBitfieldField.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtBitfieldField::ToolsQtBitfieldField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtBitfieldField::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_members = toolsTransformFieldsList(members());
    return true;
}

bool ToolsQtBitfieldField::writeImpl() const
{
    return toolsWrite();
}

ToolsQtBitfieldField::IncludesList ToolsQtBitfieldField::toolsExtraSrcIncludesImpl() const
{
    IncludesList result;
    for (auto* m : m_members) {
        assert(m != nullptr);
        auto incList = m->toolsSrcIncludes();
        result.reserve(result.size() + incList.size());
        std::move(incList.begin(), incList.end(), std::back_inserter(result));
    }

    return result;
}

std::string ToolsQtBitfieldField::toolsExtraPropsImpl() const
{
    util::StringsList props;
    props.reserve(m_members.size());
    auto prefix =
        comms::className(dslObj().name()) + strings::membersSuffixStr() +
        "::createProps_";
    for (auto* f : m_members) {
        auto str = ".add(" + prefix + comms::accessName(f->field().dslObj().name()) + "(true))";
        props.push_back(std::move(str));
    }

    return util::strListToString(props, "\n", "");  
}

std::string ToolsQtBitfieldField::toolsDefMembersImpl() const
{
    util::StringsList elems;
    for (auto* m : m_members) {
        auto members = m->toolsDefMembers();
        if (!members.empty()) {
            elems.push_back(std::move(members));
        }

        elems.push_back(m->toolsDefFunc());
        assert(!elems.back().empty());
    }

    return util::strListToString(elems, "\n", "");
}

void ToolsQtBitfieldField::toolsSetReferencedImpl()
{
    for (auto* m : m_members) {
        m->toolsSetReferenced();
    }
}



} // namespace commsdsl2tools_qt
