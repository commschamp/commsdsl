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

#include "commsdsl/gen/GenEnumField.h"

#include "commsdsl/gen/GenGenerator.h"
#include "commsdsl/gen/GenIntField.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>

namespace commsdsl
{

namespace gen
{

namespace 
{

std::uintmax_t maxTypeValueInternal(commsdsl::parse::ParseEnumField::Type val)
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
    static_assert(MapSize == static_cast<std::size_t>(commsdsl::parse::ParseEnumField::Type::NumOfValues),
            "Invalid map");

    if (commsdsl::parse::ParseEnumField::Type::NumOfValues <= val) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        val = commsdsl::parse::ParseEnumField::Type::Uint64;
    }
    return Map[static_cast<unsigned>(val)];
}

} // namespace 
    

class GenEnumFieldImpl
{
public:

    using RevValueInfo = GenEnumField::RevValueInfo;
    using SortedRevValues = GenEnumField::SortedRevValues;

    explicit GenEnumFieldImpl(commsdsl::parse::ParseEnumField dslObj) : m_dslObj(dslObj) {}

    bool prepare()
    {
        auto type = m_dslObj.type();
        m_bigUnsigned =
            (type == commsdsl::parse::ParseEnumField::Type::Uint64) ||
            (type == commsdsl::parse::ParseEnumField::Type::Uintvar);

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

    std::string adjustName(const std::string& val) const
    {
        std::string result = val;        
        adjustFirstLetterInName(result);

        auto& values = m_dslObj.values();        
        while (true) {
            if (values.find(result) == values.end()) {
                break;
            }

            result += '_';
        }        

        return result;
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
    void adjustFirstLetterInName(std::string& val) const
    {
        auto& firstElem = m_sortedRevValues.front();
        assert(firstElem.second != nullptr);
        assert(!firstElem.second->empty());
        auto firstLetter = firstElem.second->front();
        bool useLower = (std::tolower(firstLetter) == static_cast<int>(firstLetter));

        if (!useLower) {
            assert(!val.empty());
            assert(val[0] == static_cast<char>(std::toupper(val[0])));
            return;
        }        

        val[0] = static_cast<char>(std::tolower(val[0]));
    }

    commsdsl::parse::ParseEnumField m_dslObj;
    SortedRevValues m_sortedRevValues;       
    bool m_bigUnsigned = false;
};    

GenEnumField::GenEnumField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenEnumFieldImpl>(enumDslObj()))
{
    assert(dslObj.kind() == commsdsl::parse::ParseField::Kind::Enum);
}

GenEnumField::~GenEnumField() = default;

bool GenEnumField::isUnsignedUnderlyingType() const
{
    return GenIntField::isUnsignedType(enumDslObj().type());
}

unsigned GenEnumField::hexWidth() const
{
    return m_impl->hexWidth();
}

std::string GenEnumField::valueName(std::intmax_t value) const
{
    auto obj = enumDslObj();
    auto& revValues = obj.revValues();
    auto iter = revValues.find(value);
    if (iter != revValues.end()) {
        return iter->second;
    }

    return strings::emptyString();
}

std::string GenEnumField::adjustName(const std::string& val) const
{
    return m_impl->adjustName(val);
}

commsdsl::parse::ParseEnumField GenEnumField::enumDslObj() const
{
    return commsdsl::parse::ParseEnumField(dslObj());
}

const GenEnumField::SortedRevValues& GenEnumField::sortedRevValues() const
{
    return m_impl->sortedRevValues();
}

std::string GenEnumField::valueToString(std::intmax_t val) const
{
    return m_impl->valueToString(val);
}

bool GenEnumField::hasValuesLimit() const
{
    return m_impl->hasValuesLimit();
}

std::string GenEnumField::firstValueStr() const
{
    return adjustName(strings::enumFirstValueStr());
}

std::string GenEnumField::lastValueStr() const
{
    return adjustName(strings::enumLastValueStr());
}

std::string GenEnumField::valuesLimitStr() const
{
    return adjustName(strings::enumValuesLimitStr());
}

bool GenEnumField::prepareImpl()
{
    return m_impl->prepare();
}

GenEnumField::FieldRefInfo GenEnumField::processInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());

    auto obj = enumDslObj();
    auto& values = obj.values();

    FieldRefInfo info;
    auto iter = values.find(refStr);
    if (iter != values.end()) {
        info.m_field = this;
        info.m_valueName = refStr;
        info.m_refType = FieldRefType_InnerValue;
    }

    return info;    
}

} // namespace gen

} // namespace commsdsl
