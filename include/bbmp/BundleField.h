#pragma once

#include "Field.h"

namespace bbmp
{

class BundleFieldImpl;
class BBMP_API BundleField : public Field
{
    using Base = Field;
public:

    using Members = std::vector<Field>;

    explicit BundleField(const BundleFieldImpl* impl);
    explicit BundleField(Field field);

    const Members& members() const;
};

} // namespace bbmp
