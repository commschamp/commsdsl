#pragma once

#include "Field.h"

namespace bbmp
{

class ListFieldImpl;
class BBMP_API ListField : public Field
{
    using Base = Field;
public:

    explicit ListField(const ListFieldImpl* impl);
    explicit ListField(Field field);

    Field elementField() const;
    unsigned fixedCount() const;
    bool hasCountPrefixField() const;
    Field countPrefixField() const;
    bool hasLengthPrefixField() const;
    Field lengthPrefixField() const;
    bool hasElemLengthPrefixField() const;
    Field elemLengthPrefixField() const;
    bool elemFixedLength() const;
};

} // namespace bbmp
