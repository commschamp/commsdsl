#pragma once

#include <utility>
#include <map>
#include <string>

#include "Field.h"
#include "Endian.h"

namespace bbmp
{

class FloatFieldImpl;
class FloatField : public Field
{
    using Base = Field;
public:

    enum class Type
    {
        Float,
        Double,
        NumOfValues
    };
    using ValidRange = std::pair<double, double>;
    using ValidRangesList = std::vector<ValidRange>;
    using SpecialValues = std::map<std::string, double>;

    explicit FloatField(const FloatFieldImpl* impl);
    explicit FloatField(Field field);

    Type type() const;
    Endian endian() const;
    std::size_t length() const;
    double defaultValue() const;
    const ValidRangesList& validRanges() const;
    const SpecialValues& specialValues() const;
};

} // namespace bbmp
