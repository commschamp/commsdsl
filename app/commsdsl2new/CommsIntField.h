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
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsBaseClassDefImpl() const override;

private:
    std::string commsCommonHasSpecialsFuncCodeInternal() const;
    std::string commsCommonValueNamesMapCodeInternal() const;
    std::string commsCommonSpecialsCodeInternal() const;
    std::string commsCommonSpecialNamesMapCodeInternal() const;
    std::string commsFieldDefOptsInternal() const;

    void commsAddLengthOptInternal(StringsList& opts) const;
    void commsAddSerOffsetOptInternal(StringsList& opts) const;
    void commsAddScalingOptInternal(StringsList& opts) const;
    void commsAddUnitsOptInternal(StringsList& opts) const;
    void commsAddDefaultValueOptInternal(StringsList& opts) const;
    void commsAddValidRangesOptInternal(StringsList& opts) const;
    void commsAddCustomRefreshOptInternal(StringsList& opts) const;


    bool commsRequiresFailOnInvalidRefreshInternal() const;
};

} // namespace commsdsl2new
