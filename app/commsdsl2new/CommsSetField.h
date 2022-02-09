#pragma once

#include "CommsField.h"

#include "commsdsl/gen/SetField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsSetField final : public commsdsl::gen::SetField, public CommsField
{
    using Base = commsdsl::gen::SetField;
    using CommsBase = CommsField;
public:
    CommsSetField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsBaseClassDefImpl() const override;
    virtual std::string commsDefPublicCodeImpl() const override;
    virtual std::string commsDefValidFuncBodyImpl() const override;

private:
    std::string commsCommonBitNameFuncCodeInternal() const;
    std::string commsFieldDefOptsInternal() const;
    std::string commsDefBitsAccessCodeInternal() const;
    std::string commsDefBitNameFuncCodeInternal() const;

    void commsAddLengthOptInternal(commsdsl::gen::util::StringsList& opts) const;
    void commsAddDefaultValueOptInternal(commsdsl::gen::util::StringsList& opts) const;
    void commsAddReservedBitsOptInternal(commsdsl::gen::util::StringsList& opts) const;
};

} // namespace commsdsl2new
