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

#include "ToolsQtListField.h"

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

ToolsQtListField::ToolsQtListField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtListField::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    m_toolsMemberElementField = dynamic_cast<ToolsQtField*>(memberElementField());
    m_toolsExternalElementField = dynamic_cast<ToolsQtField*>(externalElementField());
    return true;
}

bool ToolsQtListField::writeImpl() const
{
    return toolsWrite();
}

ToolsQtListField::IncludesList ToolsQtListField::toolsExtraSrcIncludesImpl() const
{
    IncludesList result;
    auto addIncludes = 
        [&result](ToolsQtField* f)
        {
            if (f == nullptr) {
                return;
            }

            auto incs = f->toolsSrcIncludes();
            result.reserve(result.size() + incs.size());
            std::move(incs.begin(), incs.end(), std::back_inserter(result));
        };

    addIncludes(m_toolsMemberElementField);
    addIncludes(m_toolsExternalElementField);

    if (m_toolsExternalElementField != nullptr) {
        result.push_back(m_toolsExternalElementField->toolsRelDeclHeaderFile());
    }

    return result;
}

std::string ToolsQtListField::toolsExtraPropsImpl() const
{
    auto obj = listDslObj();
    util::StringsList props;

    std::string elemSerHiddenParam = "serHidden";
    if (obj.hasElemLengthPrefixField()) {
        elemSerHiddenParam = "true";
    }
    else {
        props.push_back(".serialisedHidden()");
    }

    auto& gen = static_cast<const ToolsQtGenerator&>(generator());
    if (m_toolsMemberElementField != nullptr) {
        auto func =
            comms::className(obj.name()) + strings::membersSuffixStr() +
            "::createProps_" + comms::accessName(m_toolsMemberElementField->field().dslObj().name()) + "(" +
            elemSerHiddenParam + ")";
        props.push_back(".add(" + func + ")");
    }
    else {
        assert(m_toolsExternalElementField != nullptr);
        auto scope = gen.getTopNamespace() + "::" + 
            comms::scopeFor(m_toolsExternalElementField->field(), gen, true, false);
        auto func = scope + "::createProps_" + comms::accessName(m_toolsExternalElementField->field().dslObj().name()) +
        "(Field::ValueType::value_type::name(), " + elemSerHiddenParam + ')';
        props.push_back(".add(" + func + ")");
    }

    do {
        if (obj.hasElemLengthPrefixField()) {
            break;
        }
        
        if ((!obj.hasCountPrefixField()) && (!obj.hasLengthPrefixField())) {
            break;
        }

        props.push_back(".prefixName(\"" + toolsPrefixNameInternal() + "\")");
        props.push_back(".showPrefix()");
    } while (false);

    props.push_back(".appendIndexToElementName()");
    return util::strListToString(props, "\n", "");
}

std::string ToolsQtListField::toolsDefMembersImpl() const
{
    if (m_toolsMemberElementField == nullptr) {
        return strings::emptyString();
    }

    util::StringsList elems;
    auto members = m_toolsMemberElementField->toolsDefMembers();
    if (!members.empty()) {
        elems.push_back(std::move(members));
    }

    elems.push_back(m_toolsMemberElementField->toolsDefFunc());
    return util::strListToString(elems, "\n", "");
}

void ToolsQtListField::toolsSetReferencedImpl()
{
    toolsUpdateFieldReferencedIfExists(m_toolsMemberElementField);
    toolsUpdateFieldReferencedIfExists(m_toolsExternalElementField);
}

std::string ToolsQtListField::toolsPrefixNameInternal() const
{
    std::string result;
    auto assignNameFunc = 
        [&result](const commsdsl::gen::Field* f)
        {
            if ((!result.empty()) || (f == nullptr)) {
                return;
            }

            result = util::displayName(f->dslObj().displayName(), f->dslObj().name());
        };

    assignNameFunc(memberCountPrefixField());
    assignNameFunc(externalCountPrefixField());
    assignNameFunc(memberLengthPrefixField());
    assignNameFunc(externalLengthPrefixField());

    return result;
}


} // namespace commsdsl2tools_qt
