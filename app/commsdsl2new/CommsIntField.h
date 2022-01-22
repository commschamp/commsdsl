#pragma once

#include "commsdsl/gen/IntField.h"

#include "CommsField.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsIntField final : public commsdsl::gen::IntField, public CommsField
{
    using Base = commsdsl::gen::IntField;
    using CommsBase = CommsField;
public:
    CommsIntField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
};

} // namespace commsdsl2new
