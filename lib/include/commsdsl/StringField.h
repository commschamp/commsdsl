#pragma once

#include "Field.h"

namespace commsdsl
{

class StringFieldImpl;
class COMMSDSL_API StringField : public Field
{
    using Base = Field;
public:

    explicit StringField(const StringFieldImpl* impl);
    explicit StringField(Field field);

    const std::string& defaultValue() const;
    const std::string& encodingStr() const;
    std::size_t fixedLength() const;
    bool hasLengthPrefixField() const;
    Field lengthPrefixField() const;
    bool hasZeroTermSuffix() const;
};

} // namespace commsdsl
