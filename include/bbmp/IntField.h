#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <map>
#include <string>

#include "Field.h"
#include "Endian.h"
#include "Protocol.h"

namespace bbmp
{

class IntFieldImpl;
class IntField : public Field
{
    using Base = Field;
public:

    enum class Type
    {
        Int8,
        Uint8,
        Int16,
        Uint16,
        Int32,
        Uint32,
        Int64,
        Uint64,
        Intvar,
        Uintvar,
        NumOfValues
    };
    using ScalingRatio = std::pair<std::intmax_t, std::intmax_t>;
    using ValidRange = std::pair<std::intmax_t, std::intmax_t>;
    using ValidRangesList = std::vector<ValidRange>;

    struct SpecialValueInfo
    {
        std::intmax_t m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = Protocol::notYetDeprecated();
    };

    using SpecialValues = std::map<std::string, SpecialValueInfo>;

    explicit IntField(const IntFieldImpl* impl);
    explicit IntField(Field field);

    Type type() const;
    Endian endian() const;
    std::size_t length() const;
    std::size_t bitLength() const;
    std::intmax_t serOffset() const;
    std::intmax_t minValue() const;
    std::intmax_t maxValue() const;
    std::intmax_t defaultValue() const;
    ScalingRatio scaling() const;
    const ValidRangesList& validRanges() const;
    const SpecialValues& specialValues() const;
};

inline
bool operator==(const IntField::SpecialValueInfo& i1, const IntField::SpecialValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

} // namespace bbmp
