#include "commsdsl/StringField.h"

#include <cassert>

#include "StringFieldImpl.h"

namespace commsdsl
{

namespace
{

const StringFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const StringFieldImpl*>(ptr);
}

} // namespace

StringField::StringField(const StringFieldImpl* impl)
  : Base(impl)
{
}

StringField::StringField(Field field)
  : Base(field)
{
    assert(kind() == Kind::String);
}

const std::string& StringField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const std::string& StringField::encodingStr() const
{
    return cast(m_pImpl)->encodingStr();
}

unsigned StringField::fixedLength() const
{
    return cast(m_pImpl)->length();
}

bool StringField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasPrefixField();
}

Field StringField::lengthPrefixField() const
{
    return cast(m_pImpl)->prefixField();
}

bool StringField::hasZeroTermSuffix() const
{
    return cast(m_pImpl)->hasZeroTermSuffix();
}

} // namespace commsdsl
