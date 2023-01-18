//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtSetField.h"

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

ToolsQtSetField::ToolsQtSetField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) :
    Base(generator, dslObj, parent),
    ToolsBase(static_cast<Base&>(*this))
{
}

bool ToolsQtSetField::writeImpl() const
{
    return toolsWrite();
}

std::string ToolsQtSetField::toolsExtraPropsImpl() const
{
    util::StringsList props;
    auto obj = setDslObj();
    auto& bits = obj.bits();
    auto& revBits = obj.revBits();
    props.reserve(bits.size());
    unsigned prevBitIdx = std::numeric_limits<unsigned>::max();
    for (auto& rBit : revBits) {
        if (prevBitIdx == rBit.first) {
            continue;
        }

        auto iter = bits.find(rBit.second);
        assert(iter != bits.end());
        auto& b = *iter;

        if (!generator().doesElementExist(b.second.m_sinceVersion, b.second.m_deprecatedSince, true)) {
            continue;
        }

        prevBitIdx = rBit.first;

        auto* bitName = &b.second.m_displayName;
        if (bitName->empty()) {
            bitName = &b.first;
        }
        props.push_back(".add(" + util::numToString(rBit.first) + ", \"" + *bitName + "\")");
    }
    return util::strListToString(props, "\n", ""); 
}

} // namespace commsdsl2tools_qt
