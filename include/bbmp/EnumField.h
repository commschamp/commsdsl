#pragma once

#include "IntField.h"

namespace bbmp
{

class EnumFieldImpl;
class EnumField : public Field
{
    using Base = Field;
public:

    using Type = IntField::Type;
    using Values = std::map<std::string, std::intmax_t>;
    using RevValues = std::multimap<std::intmax_t, std::string>;
    using ValidRange = std::pair<std::intmax_t, std::intmax_t>;
    using ValidRangesList = std::vector<ValidRange>;

    explicit EnumField(const EnumFieldImpl* impl);
    explicit EnumField(Field field);

    Type type() const;
    Endian endian() const;
    std::size_t length() const;
    std::size_t bitLength() const;
    std::intmax_t defaultValue() const;
    const Values& values() const;
    const RevValues& revValues() const;
    const ValidRangesList& validRanges() const;
    bool isNonUniqueAllowed() const;
    bool isUnique() const;
};

} // namespace bbmp
