#pragma once

#include "commsdsl/gen/Elem.h"

namespace commsdsl2new
{

class CommsField
{
public:
    static bool isWritable(const commsdsl::gen::Elem* parent);
};

} // namespace commsdsl2new
