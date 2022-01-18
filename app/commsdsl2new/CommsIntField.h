#pragma once

#include "commsdsl/gen/IntField.h"

#include "CommsField.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsIntField final : public commsdsl::gen::IntField, public CommsField
{
    using Base = commsdsl::gen::IntField;
public:
    CommsIntField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // virtual std::string genCodeImpl();
    virtual bool writeImpl();    
};

} // namespace commsdsl2new
