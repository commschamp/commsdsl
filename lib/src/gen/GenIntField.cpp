//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenIntField.h"

#include "commsdsl/gen/GenGenerator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class GenIntFieldImpl
{
public:
    using ParseIntField = GenIntField::ParseIntField;
    using GenSpecialsList = GenIntField::GenSpecialsList;

    bool genPrepare(ParseIntField parseObj)
    {
        auto& dslSpecials = parseObj.parseSpecialValues();
        m_specialsSorted.reserve(dslSpecials.size());
        for (auto& s : dslSpecials) {
            m_specialsSorted.emplace_back(s.first, s.second);
        }

        bool compareUnsigned = GenIntField::genIsUnsignedType(parseObj.parseType());
        std::sort(
            m_specialsSorted.begin(), m_specialsSorted.end(),
            [compareUnsigned](auto& elem1, auto& elem2)
            {
                if (elem1.second.m_value == elem2.second.m_value) {
                    return elem1.first < elem2.first;
                }

                if (compareUnsigned) {
                    return static_cast<std::uintmax_t>(elem1.second.m_value) < static_cast<std::uintmax_t>(elem2.second.m_value);
                }

                return elem1.second.m_value < elem2.second.m_value;
            });

        return true;
    }

    const GenSpecialsList& genSpecialsSortedByValue() const
    {
        return m_specialsSorted;
    }

private:
    GenSpecialsList m_specialsSorted;
};

GenIntField::GenIntField(GenGenerator& generator, commsdsl::parse::ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenIntFieldImpl>())
{
    assert(parseObj.parseKind() == commsdsl::parse::ParseField::ParseKind::Int);
}

GenIntField::~GenIntField() = default;

bool GenIntField::genIsUnsignedType(ParseIntField::ParseType value)
{
    static const ParseIntField::ParseType Map[] = {
        ParseIntField::ParseType::Uint8,
        ParseIntField::ParseType::Uint16,
        ParseIntField::ParseType::Uint32,
        ParseIntField::ParseType::Uint64,
        ParseIntField::ParseType::Uintvar,
    };

    auto iter = std::find(std::begin(Map), std::end(Map), value);
    return iter != std::end(Map);
}

bool GenIntField::genIsUnsignedType() const
{
    return genIsUnsignedType(genIntFieldParseObj().parseType());
}

const GenIntField::GenSpecialsList& GenIntField::genSpecialsSortedByValue() const
{
    return m_impl->genSpecialsSortedByValue();
}

bool GenIntField::genPrepareImpl()
{
    return m_impl->genPrepare(genIntFieldParseObj());
}

GenIntField::GenFieldRefInfo GenIntField::genProcessInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());
    auto obj = genIntFieldParseObj();
    auto& specials = obj.parseSpecialValues();

    GenFieldRefInfo info;
    auto iter = specials.find(refStr);
    if (iter != specials.end()) {
        info.m_field = this;
        info.m_valueName = refStr;
        info.m_refType = FieldRefType_InnerValue;
    }

    return info;
}

GenIntField::ParseIntField GenIntField::genIntFieldParseObj() const
{
    return ParseIntField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
