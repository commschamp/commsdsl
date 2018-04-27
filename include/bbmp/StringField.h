#pragma once

#include "Field.h"

namespace bbmp
{

class StringFieldImpl;
class BBMP_API StringField : public Field
{
    using Base = Field;
public:

    explicit StringField(const StringFieldImpl* impl);
    explicit StringField(Field field);

    const std::string& defaultValue() const;
    const std::string& encodingStr() const;
    unsigned fixedLength() const;
    bool hasLengthPrefixField() const;
    Field lengthPrefixField() const;
    bool hasZeroTermSuffix() const;
};

} // namespace bbmp
