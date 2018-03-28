#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <map>
#include <string>

#include "Field.h"
#include "Endian.h"

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
    using SpecialValues = std::map<std::string, std::intmax_t>;

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

} // namespace bbmp
