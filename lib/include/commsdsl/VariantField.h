#pragma once

#include "Field.h"

namespace commsdsl
{

class VariantFieldImpl;
class COMMSDSL_API VariantField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;

    explicit VariantField(const VariantFieldImpl* impl);
    explicit VariantField(Field field);

    Members members() const;
    std::size_t defaultMemberIdx() const;
    bool displayIdxReadOnlyHidden() const;

};

} // namespace commsdsl
