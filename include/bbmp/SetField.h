#pragma once

#include "IntField.h"

namespace bbmp
{

class SetFieldImpl;
class SetField : public Field
{
    using Base = Field;
public:

    using Type = IntField::Type;
    using Bits = std::map<std::string, unsigned>;
    using RevBits = std::multimap<unsigned, std::string>;

    explicit SetField(const SetFieldImpl* impl);
    explicit SetField(Field field);

    Type type() const;
    Endian endian() const;
    std::size_t length() const;
    std::size_t bitLength() const;
    std::uint64_t defaultValue() const;
    const Bits& bits() const;
    const RevBits& revBits() const;
    std::uint64_t reservedBits() const;
    std::uint64_t reservedValue() const;
    bool isNonUniqueAllowed() const;
    bool isUnique() const;
};

} // namespace bbmp
