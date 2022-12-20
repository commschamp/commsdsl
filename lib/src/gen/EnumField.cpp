//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/EnumField.h"

#include "commsdsl/gen/Generator.h"
#include "commsdsl/gen/IntField.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace gen
{

namespace 
{

std::uintmax_t maxTypeValueInternal(commsdsl::parse::EnumField::Type val)
{
    static const std::uintmax_t Map[] = {
        /* Int8 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int8_t>::max()),
        /* Uint8 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint8_t>::max()),
        /* Int16 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int16_t>::max()),
        /* Uint16 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint16_t>::max()),
        /* Int32 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int32_t>::max()),
        /* Uint32 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint32_t>::max()),
        /* Int64 */ static_cast<std::uintmax_t>(std::numeric_limits<std::int64_t>::max()),
        /* Uint64 */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint64_t>::max()),
        /* Intvar */ static_cast<std::uintmax_t>(std::numeric_limits<std::int64_t>::max()),
        /* Uintvar */ static_cast<std::uintmax_t>(std::numeric_limits<std::uint64_t>::max())
    };
    static const std::size_t MapSize =
            std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::EnumField::Type::NumOfValues),
            "Invalid map");

    if (commsdsl::parse::EnumField::Type::NumOfValues <= val) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        val = commsdsl::parse::EnumField::Type::Uint64;
    }
    return Map[static_cast<unsigned>(val)];
}

} // namespace 
    

class EnumFieldImpl
{
public:

    using RevValueInfo = EnumField::RevValueInfo;
    using SortedRevValues = EnumField::SortedRevValues;

    explicit EnumFieldImpl(commsdsl::parse::EnumField dslObj) : m_dslObj(dslObj) {}

    bool prepare()
    {
        auto type = m_dslObj.type();
        m_bigUnsigned =
            (type == commsdsl::parse::EnumField::Type::Uint64) ||
            (type == commsdsl::parse::EnumField::Type::Uintvar);

        for (auto& v : m_dslObj.revValues()) {
            m_sortedRevValues.push_back(std::make_pair(v.first, &v.second));
        }

        if (m_bigUnsigned) {
            std::sort(
                m_sortedRevValues.begin(), m_sortedRevValues.end(),
                [](const auto& elem1, const auto& elem2) -> bool
                {
                    return static_cast<std::uintmax_t>(elem1.first) < static_cast<std::uintmax_t>(elem2.first);
                });
        }

        return true;
    }

    unsigned hexWidth() const
    {
        std::uintmax_t hexWidth = 0U;
        if (m_dslObj.hexAssign()) {
            hexWidth = m_dslObj.maxLength() * 2U;
        }
        return static_cast<unsigned>(hexWidth);
    }

    std::string valueToString(std::intmax_t val) const
    {
        unsigned hexW = hexWidth();

        if ((m_bigUnsigned) || (0U < hexW)) {
            return util::numToString(static_cast<std::uintmax_t>(val), hexW);
        }

        return util::numToString(val);
    }    

    bool hasValuesLimit() const    
    {
        auto maxTypeValue = maxTypeValueInternal(m_dslObj.type());
        if (m_bigUnsigned) {
            return static_cast<std::uintmax_t>(m_sortedRevValues.back().first) < maxTypeValue;
        }

        return m_sortedRevValues.back().first < static_cast<std::intmax_t>(maxTypeValue);        
    }

    const SortedRevValues& sortedRevValues() const
    {
        return m_sortedRevValues;
    }

private:
    commsdsl::parse::EnumField m_dslObj;
    SortedRevValues m_sortedRevValues;       
    bool m_bigUnsigned = false;
};    

EnumField::EnumField(Generator& generator, commsdsl::parse::Field dslObj, Elem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<EnumFieldImpl>(enumDslObj()))
{
    assert(dslObj.kind() == commsdsl::parse::Field::Kind::Enum);
}

EnumField::~EnumField() = default;

bool EnumField::isUnsignedUnderlyingType() const
{
    return IntField::isUnsignedType(enumDslObj().type());
}

unsigned EnumField::hexWidth() const
{
    return m_impl->hexWidth();
}

std::string EnumField::valueName(std::intmax_t value) const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    auto iter = revValues.find(value);
    if (iter != revValues.end()) {
        return iter->second;
    }

    return strings::emptyString();
}

commsdsl::parse::EnumField EnumField::enumDslObj() const
{
    return commsdsl::parse::EnumField(dslObj());
}

const EnumField::SortedRevValues& EnumField::sortedRevValues() const
{
    return m_impl->sortedRevValues();
}

std::string EnumField::valueToString(std::intmax_t val) const
{
    return m_impl->valueToString(val);
}

bool EnumField::hasValuesLimit() const
{
    return m_impl->hasValuesLimit();
}

bool EnumField::prepareImpl()
{
    return m_impl->prepare();
}

} // namespace gen

} // namespace commsdsl
