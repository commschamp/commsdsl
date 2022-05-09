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

#include "ToolsQtIntField.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtIntField::ToolsQtIntField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtIntField::writeImpl() const
{
    return toolsWrite();
}

std::string ToolsQtIntField::toolsExtraPropsImpl() const
{
    util::StringsList props;
    auto obj = Base::intDslObj();
    auto decimals = obj.displayDecimals();
    auto offset = obj.displayOffset();
    if (decimals != 0U) {
        props.push_back(".scaledDecimals(" + util::numToString(decimals) + ')');
    }

    if (offset != 0) {
        props.push_back(".displayOffset(" + util::numToString(offset) + ')');
    }

    auto& specials = specialsSortedByValue();
    if (!specials.empty() && (obj.displaySpecials())) {
        auto type = obj.type();
        bool bigUnsigned =
            (type == commsdsl::parse::IntField::Type::Uint64) ||
            (type == commsdsl::parse::IntField::Type::Uintvar);

        auto addSpecDisplayNameFunc =
            [&props, bigUnsigned](std::intmax_t val, const std::string& name, const std::string& displayName)
            {
                auto valStr = util::numToString(val);
                if (bigUnsigned) {
                    valStr = "static_cast<long long>(" + util::numToString(static_cast<std::uintmax_t>(val)) + ")";
                }

                auto* nameToAdd = &displayName;
                if (nameToAdd->empty()) {
                    nameToAdd = &name;
                }

                props.push_back(".addSpecial(\"" + *nameToAdd + "\", " + valStr + ")");
            };

        for (auto& s : specials) {
            addSpecDisplayNameFunc(s.second.m_value, s.first, s.second.m_displayName);
        }
    }

    return util::strListToString(props, "\n", "");    
}


} // namespace commsdsl2tools_qt
