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

std::uintmax_t genMaxTypeValueInternal(commsdsl::parse::ParseEnumField::Type val)
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
    using ParseEnumField = GenEnumField::ParseEnumField;
    using RevValueInfo = GenEnumField::RevValueInfo;
    using SortedRevValues = GenEnumField::SortedRevValues;

    explicit GenEnumFieldImpl(ParseEnumField dslObj) : m_dslObj(dslObj) {}

    bool genPrepare()
    {
        auto type = m_dslObj.parseType();
        m_bigUnsigned =
            (type == ParseEnumField::Type::Uint64) ||
            (type == ParseEnumField::Type::Uintvar);

        for (auto& v : m_dslObj.parseRevValues()) {
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

    unsigned genHexWidth() const
    {
        std::uintmax_t hexWidth = 0U;
        if (m_dslObj.parseHexAssign()) {
            hexWidth = m_dslObj.parseMaxLength() * 2U;
        }
        return static_cast<unsigned>(hexWidth);
    }

    std::string genAdjustName(const std::string& val) const
    {
        std::string result = val;        
        genAdjustFirstLetterInName(result);

        auto& values = m_dslObj.parseValues();        
        while (true) {
            if (values.find(result) == values.end()) {
                break;
            }

            result += '_';
        }        

        return result;
    }

    std::string genValueToString(std::intmax_t val) const
    {
        unsigned hexW = genHexWidth();

        if ((m_bigUnsigned) || (0U < hexW)) {
            return util::genNumToString(static_cast<std::uintmax_t>(val), hexW);
        }

        return util::genNumToString(val);
    }    

    bool genHasValuesLimit() const    
    {
        auto maxTypeValue = genMaxTypeValueInternal(m_dslObj.parseType());
        if (m_bigUnsigned) {
            return static_cast<std::uintmax_t>(m_sortedRevValues.back().first) < maxTypeValue;
        }

        return m_sortedRevValues.back().first < static_cast<std::intmax_t>(maxTypeValue);        
    }

    const SortedRevValues& genSortedRevValues() const
    {
        return m_sortedRevValues;
    }

private:
    void genAdjustFirstLetterInName(std::string& val) const
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

    ParseEnumField m_dslObj;
    SortedRevValues m_sortedRevValues;       
    bool m_bigUnsigned = false;
};    

GenEnumField::GenEnumField(GenGenerator& generator, commsdsl::parse::ParseField dslObj, GenElem* parent) :
    Base(generator, dslObj, parent),
    m_impl(std::make_unique<GenEnumFieldImpl>(genEnumFieldParseObj()))
{
    assert(dslObj.parseKind() == commsdsl::parse::ParseField::Kind::Enum);
}

GenEnumField::~GenEnumField() = default;

bool GenEnumField::genIsUnsignedUnderlyingType() const
{
    return GenIntField::genIsUnsignedType(genEnumFieldParseObj().parseType());
}

unsigned GenEnumField::genHexWidth() const
{
    return m_impl->genHexWidth();
}

std::string GenEnumField::genValueName(std::intmax_t value) const
{
    auto obj = genEnumFieldParseObj();
    auto& revValues = obj.parseRevValues();
    auto iter = revValues.find(value);
    if (iter != revValues.end()) {
        return iter->second;
    }

    return strings::genEmptyString();
}

std::string GenEnumField::genAdjustName(const std::string& val) const
{
    return m_impl->genAdjustName(val);
}

GenEnumField::ParseEnumField GenEnumField::genEnumFieldParseObj() const
{
    return ParseEnumField(genParseObj());
}

const GenEnumField::SortedRevValues& GenEnumField::genSortedRevValues() const
{
    return m_impl->genSortedRevValues();
}

std::string GenEnumField::genValueToString(std::intmax_t val) const
{
    return m_impl->genValueToString(val);
}

bool GenEnumField::genHasValuesLimit() const
{
    return m_impl->genHasValuesLimit();
}

std::string GenEnumField::genFirstValueStr() const
{
    return genAdjustName(strings::genEnumFirstValueStr());
}

std::string GenEnumField::genLastValueStr() const
{
    return genAdjustName(strings::genEnumLastValueStr());
}

std::string GenEnumField::genValuesLimitStr() const
{
    return genAdjustName(strings::genEnumValuesLimitStr());
}

bool GenEnumField::genPrepareImpl()
{
    return m_impl->genPrepare();
}

GenEnumField::FieldRefInfo GenEnumField::genProcessInnerRefImpl(const std::string& refStr) const
{
    assert(!refStr.empty());

    auto obj = genEnumFieldParseObj();
    auto& values = obj.parseValues();

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
