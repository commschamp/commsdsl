#pragma once

#include "IntField.h"

namespace commsdsl
{

class SetFieldImpl;
class COMMSDSL_API SetField : public Field
{
    using Base = Field;
public:

    using Type = IntField::Type;
    struct BitInfo
    {
        unsigned m_idx = 0U;
        unsigned m_sinceVersion = 0U;
        unsigned m_deprecatedSince = commsdsl::Protocol::notYetDeprecated();
        bool m_defaultValue = false;
        bool m_reserved = false;
        bool m_reservedValue = false;
    };

    using Bits = std::map<std::string, BitInfo>;
    using RevBits = std::multimap<unsigned, std::string>;

    explicit SetField(const SetFieldImpl* impl);
    explicit SetField(Field field);

    Type type() const;
    Endian endian() const;
    bool defaultBitValue() const;
    bool reservedBitValue() const;
    const Bits& bits() const;
    const RevBits& revBits() const;
    bool isNonUniqueAllowed() const;
    bool isUnique() const;
    bool validCheckVersion() const;
};

inline
bool operator==(const SetField::BitInfo& i1, const SetField::BitInfo& i2)
{
    return (i1.m_idx == i2.m_idx) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const SetField::BitInfo& i1, const SetField::BitInfo& i2)
{
    return !(i1 == i2);
}


} // namespace commsdsl
