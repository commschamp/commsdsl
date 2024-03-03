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

#include "ToolsQtFloatField.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cmath>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtFloatField::ToolsQtFloatField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtFloatField::writeImpl() const
{
    return toolsWrite();
}

std::string ToolsQtFloatField::toolsExtraPropsImpl() const
{
    util::StringsList props;
    auto obj = floatDslObj();
    auto decimals = obj.displayDecimals();
    if (decimals != 0U) {
        props.push_back(".decimals(" + util::numToString(decimals) + ")");
    }

    do {
        if (!obj.displaySpecials()) {
            break;
        }

        auto& specials = specialsSortedByValue();
        if (specials.empty()) {
            break;
        }

        auto addSpecDisplayNameFunc =
            [&props](double val, const std::string& name, const std::string& displayName)
            {
                std::string valStr;
                if (std::isnan(val)) {
                    valStr = "std::numeric_limits<double>::quiet_NaN()";
                }
                else if (std::isinf(val)) {
                    valStr = "std::numeric_limits<double>::infinity()";
                    if (val < 0.0) {
                        valStr = '-' + valStr;
                    }
                }
                else {
                    valStr = std::to_string(val);
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

    } while (false);

    return util::strListToString(props, "\n", "");
}

} // namespace commsdsl2tools_qt
