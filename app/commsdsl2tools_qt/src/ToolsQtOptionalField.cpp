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

#include "ToolsQtOptionalField.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtOptionalField::ToolsQtOptionalField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtOptionalField::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_toolsMemberField = dynamic_cast<ToolsQtField*>(memberField());
    m_toolsExternalField = dynamic_cast<ToolsQtField*>(externalField());
    if (m_toolsExternalField != nullptr) {
        m_toolsExternalField->toolsSetReferenced();
    }
    return true;
}

bool ToolsQtOptionalField::writeImpl() const
{
    return toolsWrite();
}

ToolsQtOptionalField::IncludesList ToolsQtOptionalField::toolsExtraSrcIncludesImpl() const
{
    IncludesList result;

    auto addIncludes = 
        [&result](ToolsQtField* f)
        {
            if (f == nullptr) {
                return;
            }

            if (comms::isGlobalField(f->field())) {
                result.push_back(f->relDeclHeaderFile());
            }

            auto incs = f->toolsSrcIncludes();
            result.reserve(result.size() + incs.size());
            std::move(incs.begin(), incs.end(), std::back_inserter(result));
        };

    addIncludes(m_toolsMemberField);
    addIncludes(m_toolsExternalField);
    return result;
}

std::string ToolsQtOptionalField::toolsExtraPropsImpl() const
{
    util::StringsList options;
    auto obj = optionalDslObj();
    if (obj.cond().valid() || obj.externalModeCtrl()) {
        options.push_back(".uncheckable()");
    }

    if (m_toolsMemberField != nullptr) {
        auto prefix =
            comms::className(dslObj().name()) + strings::membersSuffixStr() +
            "::createProps_";

        auto str = ".field(" + prefix + comms::accessName(m_toolsMemberField->field().dslObj().name()) + "(serHidden))";
        options.push_back(std::move(str));
        return util::strListToString(options, "\n", "");
    }

    assert(m_toolsExternalField != nullptr);
    
    auto dispName = util::displayName(m_toolsExternalField->field().dslObj().displayName(), m_toolsExternalField->field().dslObj().name());

    auto& gen = generator();
    auto str =
        ".field(" +
        gen.getTopNamespace() + "::" + comms::scopeFor(m_toolsExternalField->field(), gen, true, false) + "::"
        "createProps_" + comms::accessName(m_toolsExternalField->field().dslObj().name()) + "(\"" +
        dispName + "\", serHidden))";
    options.push_back(std::move(str));
    return util::strListToString(options, "\n", "");
}

std::string ToolsQtOptionalField::toolsDefMembersImpl() const
{
    if (m_toolsMemberField != nullptr) {
        util::StringsList elems;
        auto members = m_toolsMemberField->toolsDefMembers();
        if (!members.empty()) {
            elems.push_back(std::move(members));
        } 

        elems.push_back(m_toolsMemberField->toolsDefFunc());
        return util::strListToString(elems, "\n", "");
    }

    return strings::emptyString();
}


} // namespace commsdsl2tools_qt
