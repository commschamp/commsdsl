#pragma once

#include <utility>
#include <map>
#include <string>

#include "Field.h"
#include "Endian.h"
#include "Protocol.h"

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

    struct ValidRangeInfo
    {
        double m_min = 0.0;
        double m_max = 0.0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = Protocol::notYetDeprecated();
    };
    using ValidRangesList = std::vector<ValidRangeInfo>;

    struct SpecialValueInfo
    {
        double m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = Protocol::notYetDeprecated();
    };
    using SpecialValues = std::map<std::string, SpecialValueInfo>;

    explicit FloatField(const FloatFieldImpl* impl);
    explicit FloatField(Field field);

    Type type() const;
    Endian endian() const;
    std::size_t length() const;
    double defaultValue() const;
    const ValidRangesList& validRanges() const;
    const SpecialValues& specialValues() const;
};

inline
bool operator==(const FloatField::SpecialValueInfo& i1, const FloatField::SpecialValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const FloatField::SpecialValueInfo& i1, const FloatField::SpecialValueInfo& i2)
{
    return !(i1 == i2);
}

inline
bool operator==(const FloatField::ValidRangeInfo& i1, const FloatField::ValidRangeInfo& i2)
{
    return (i1.m_min == i2.m_min) &&
           (i1.m_max == i2.m_max) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const FloatField::ValidRangeInfo& i1, const FloatField::ValidRangeInfo& i2)
{
    return !(i1 == i2);
}

} // namespace bbmp
