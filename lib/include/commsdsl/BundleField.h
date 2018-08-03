#pragma once

#include "Field.h"

namespace commsdsl
{

class BundleFieldImpl;
class COMMSDSL_API BundleField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;

    explicit BundleField(const BundleFieldImpl* impl);
    explicit BundleField(Field field);

    Members members() const;
};

} // namespace commsdsl
