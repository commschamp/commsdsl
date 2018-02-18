#pragma once

#include "FieldImpl.h"

namespace bbmp
{

class IntFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    IntFieldImpl(::xmlNodePtr node, Logger& logger);
};

} // namespace bbmp
