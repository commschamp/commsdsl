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

#include "commsdsl/gen/GenFloatField.h"

#include "commsdsl/gen/GenGenerator.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace commsdsl
{

namespace gen
{

class GenFloatFieldImpl
{
public:
    using SpecialsList = GenFloatField::SpecialsList;
    using ParseFloatField = GenFloatField::ParseFloatField;

    bool genPrepare(ParseFloatField parseObj)
    {
        auto& dslSpecials = parseObj.parseSpecialValues();
        m_specialsSorted.reserve(dslSpecials.size());
        for (auto& s : dslSpecials) {
            m_specialsSorted.emplace_back(s.first, s.second);
        }

        std::sort(
            m_specialsSorted.begin(), m_specialsSorted.end(),
            [](auto& elem1, auto& elem2)
            {
                auto compNames =
                    [&elem1, &elem2]()
                    {
                        return elem1.first < elem2.first;
                    };

                if (std::isnan(elem1.second.m_value)) {
                    if (std::isnan(elem2.second.m_value)) {
                        return compNames();
                    }

                    return false;
                }

                if (std::isnan(elem2.second.m_value)) {
                    return true;
                }

                auto isPositiveInf =
                    [](double val)
                    {
                        return std::isinf(val) && (val > 0.0);
                    };

                if (isPositiveInf(elem1.second.m_value)) {
                    if (isPositiveInf(elem2.second.m_value)) {
                        return compNames();
                    }

                    return false;
                }

                auto isNegativeInf =
                    [](double val)
                    {
                        return std::isinf(val) && (val < 0.0);
                    };

                if (isNegativeInf(elem1.second.m_value)) {
                    if (isNegativeInf(elem2.second.m_value)) {
                        return compNames();
                    }

                    return true;
                }

                if (elem1.second.m_value != elem2.second.m_value) {
                    return elem1.second.m_value < elem2.second.m_value;
                }

                return compNames();
            });   

        return true;
    }

    const SpecialsList& genSpecialsSortedByValue() const
    {
        return m_specialsSorted;
    }

private:
    SpecialsList m_specialsSorted;
};       

GenFloatField::GenFloatField(GenGenerator& generator, ParseField parseObj, GenElem* parent) :
    Base(generator, parseObj, parent),
    m_impl(std::make_unique<GenFloatFieldImpl>())
{
    assert(parseObj.parseKind() == ParseField::Kind::Float);
}

GenFloatField::~GenFloatField() = default;

const GenFloatField::SpecialsList& GenFloatField::genSpecialsSortedByValue() const
{
    return m_impl->genSpecialsSortedByValue();
}

bool GenFloatField::genPrepareImpl()
{
    return m_impl->genPrepare(genFloatFieldParseObj());
}

GenFloatField::FieldRefInfo GenFloatField::genProcessInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());
    auto obj = genFloatFieldParseObj();
    auto& specials = obj.parseSpecialValues();

    FieldRefInfo info;
    auto iter = specials.find(refStr);
    if (iter != specials.end()) {
        info.m_field = this;
        info.m_valueName = refStr;
        info.m_refType = FieldRefType_InnerValue;
    }

    return info;    
}

GenFloatField::ParseFloatField GenFloatField::genFloatFieldParseObj() const
{
    return ParseFloatField(genParseObj());
}

} // namespace gen

} // namespace commsdsl
