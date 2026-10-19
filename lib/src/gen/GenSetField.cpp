//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

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

#include "commsdsl/gen/GenSetField.h"

#include "commsdsl/gen/GenGenerator.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace gen
{

GenSetField::GenSetField(GenGenerator& generator, commsdsl::parse::ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent)
{
    assert(parseObj.parseKind() == ParseField::ParseKind::Set);
}

GenSetField::~GenSetField() = default;

GenSetField::GenFieldRefInfo GenSetField::genProcessInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());

    auto obj = genSetFieldParseObj();
    auto& bits = obj.parseBits();
    auto& masks = obj.parseMasks();

    GenFieldRefInfo info;
    if ((bits.find(refStr) != bits.end()) ||
        (masks.find(refStr) != masks.end())) {
        info.m_field = this;
        info.m_valueName = refStr;
        info.m_refType = FieldRefType_InnerValue;
    }

    return info;
}

GenSetField::ParseSetField GenSetField::genSetFieldParseObj() const
{
    return ParseSetField(genParseObj());
}

bool GenSetField::genDoesBitExist(const std::string& name) const
{
    auto parseObj = genSetFieldParseObj();
    auto& bits = parseObj.parseBits();
    auto iter = bits.find(name);
    assert(iter != bits.end()); // Must be present if parsing succesful
    if (iter == bits.end()) {
        return false;
    }

    return genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, false);
}

bool GenSetField::genDoesMaskExist(const std::string& name) const
{
    auto parseObj = genSetFieldParseObj();
    auto& masks = parseObj.parseMasks();
    auto iter = masks.find(name);
    assert(iter != masks.end()); // Must be present if parsing succesful
    if (iter == masks.end()) {
        return false;
    }

    auto& bits = iter->second.m_bits;
    return
        std::any_of(
            bits.begin(), bits.end(),
            [this](auto& b)
            {
                return genDoesBitExist(b);
            });
}

} // namespace gen

} // namespace commsdsl
