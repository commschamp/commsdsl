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

#include "ToolsQtEnumField.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

ToolsQtEnumField::ToolsQtEnumField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtEnumField::writeImpl() const
{
    return toolsWrite();
}

std::string ToolsQtEnumField::toolsExtraPropsImpl() const
{
    util::StringsList props;
    auto obj = enumDslObj();
    auto type = obj.type();
    bool bigUnsigned =
        (type == commsdsl::parse::EnumField::Type::Uint64) ||
        (type == commsdsl::parse::EnumField::Type::Uintvar);

    using RevValueInfo = std::pair<std::intmax_t, const std::string*>;
    using SortedRevValues = std::vector<RevValueInfo>;
    SortedRevValues sortedRevValues;
    for (auto& v : obj.revValues()) {
        sortedRevValues.push_back(std::make_pair(v.first, &v.second));
    }

    if (bigUnsigned) {
        std::sort(
            sortedRevValues.begin(), sortedRevValues.end(),
            [](const auto& elem1, const auto& elem2) -> bool
            {
                return static_cast<std::uintmax_t>(elem1.first) < static_cast<std::uintmax_t>(elem2.first);
            });
    }

    auto& values = obj.values();

    props.reserve(sortedRevValues.size());
    std::intmax_t prevValue = 0;
    bool prevValueValid = false;
    for (auto& rVal : sortedRevValues) {
        if ((prevValueValid) && (prevValue == rVal.first)) {
            continue;
        }

        auto iter = values.find(*rVal.second);
        assert(iter != values.end());
        auto& v = *iter;

        if (!generator().doesElementExist(v.second.m_sinceVersion, v.second.m_deprecatedSince, true)) {
            continue;
        }

        prevValueValid = true;
        prevValue = rVal.first;

        auto* valName = &v.second.m_displayName;
        if (valName->empty()) {
            valName = &v.first;
        }
        props.push_back(".add(\"" + *valName + "\", " + util::numToString(v.second.m_value) + ")");
    }
    return util::strListToString(props, "\n", "");   
}


} // namespace commsdsl2tools_qt
