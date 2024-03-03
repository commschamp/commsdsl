//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtVariantField.h"

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

ToolsQtVariantField::ToolsQtVariantField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtVariantField::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_members = toolsTransformFieldsList(members());
    return true;
}

bool ToolsQtVariantField::writeImpl() const
{
    return toolsWrite();
}

ToolsQtVariantField::IncludesList ToolsQtVariantField::toolsExtraSrcIncludesImpl() const
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

std::string ToolsQtVariantField::toolsExtraPropsImpl() const
{
    util::StringsList props;
    props.reserve(m_members.size());
    auto prefix =
        comms::className(dslObj().name()) + strings::membersSuffixStr() +
        "::createProps_";
    for (auto* f : m_members) {
        auto str = ".add(" + prefix + comms::accessName(f->field().dslObj().name()) + "(serHidden))";
        props.push_back(std::move(str));
    }

    auto obj = variantDslObj();
    if (obj.displayIdxReadOnlyHidden()) {
        props.push_back(".setIndexHidden()");
    }    

    props.push_back(".serialisedHidden()");
    return util::strListToString(props, "\n", "");  
}

std::string ToolsQtVariantField::toolsDefMembersImpl() const
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

} // namespace commsdsl2tools_qt
