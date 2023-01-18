//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/IntField.h"
#include "commsdsl/gen/Generator.h"

#include <cassert>
#include <algorithm>

namespace commsdsl
{

namespace gen
{

class IntFieldImpl
{
public:
    using SpecialsList = IntField::SpecialsList;

    bool prepare(commsdsl::parse::IntField dslObj)
    {
        auto& dslSpecials = dslObj.specialValues();
        m_specialsSorted.reserve(dslSpecials.size());
        for (auto& s : dslSpecials) {
            m_specialsSorted.emplace_back(s.first, s.second);
        }

        bool compareUnsigned = IntField::isUnsignedType(dslObj.type());
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

    const SpecialsList& specialsSortedByValue() const
    {
        return m_specialsSorted;
    }

private:
    SpecialsList m_specialsSorted;
};   

IntField::IntField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<IntFieldImpl>())
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Int);
}

IntField::~IntField() = default;

bool IntField::isUnsignedType(commsdsl::parse::IntField::Type value)
{
    static const commsdsl::parse::IntField::Type Map[] = {
        commsdsl::parse::IntField::Type::Uint8,
        commsdsl::parse::IntField::Type::Uint16,
        commsdsl::parse::IntField::Type::Uint32,
        commsdsl::parse::IntField::Type::Uint64,
        commsdsl::parse::IntField::Type::Uintvar,
    };

    auto iter = std::find(std::begin(Map), std::end(Map), value);
    return iter != std::end(Map);
}

bool IntField::isUnsignedType() const
{
    return isUnsignedType(intDslObj().type());
}

const IntField::SpecialsList& IntField::specialsSortedByValue() const
{
    return m_impl->specialsSortedByValue();
}

bool IntField::prepareImpl()
{
    return m_impl->prepare(intDslObj());
}

commsdsl::parse::IntField IntField::intDslObj() const
{
    return commsdsl::parse::IntField(dslObj());
}

} // namespace gen

} // namespace commsdsl
