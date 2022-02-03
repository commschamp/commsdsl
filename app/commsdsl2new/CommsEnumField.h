#pragma once

#include "CommsField.h"

#include "commsdsl/gen/EnumField.h"
#include "commsdsl/gen/util.h"

#include <vector>

namespace commsdsl2new
{

class CommsGenerator;
class CommsEnumField final : public commsdsl::gen::EnumField, public CommsField
{
    using Base = commsdsl::gen::EnumField;
    using CommsBase = CommsField;
public:
    CommsEnumField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

    commsdsl::gen::util::StringsList commsEnumValues() const;

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonCodeExtraImpl() const override;
    // virtual IncludesList commsDefIncludesImpl() const override;
    // virtual std::string commsBaseClassDefImpl() const override;
    // virtual std::string commsDefPublicCodeImpl() const override;

private:
    struct RangeInfo
    {
        std::intmax_t m_min = 0;
        std::intmax_t m_max = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = commsdsl::parse::Protocol::notYetDeprecated();
    };

    using ValidRangesList = std::vector<RangeInfo>;

    bool commsPrepareValidRangesInternal();
    bool commsIsDirectValueNameMappingInternal() const;
    std::string commsCommonEnumInternal() const;
    std::string commsCommonValueNameMapInternal() const;
    std::string commsCommonValueNameFuncCodeInternal() const;
    const std::string& commsCommonValueNameDirectBodyInternal() const;
    const std::string& commsCommonValueNameBinSearchBodyInternal() const;
    std::string commsCommonValueNamesMapFuncCodeInternal() const;
    std::string commsCommonValueNamesMapDirectBodyInternal() const;
    std::string commsCommonValueNamesMapBinSearchBodyInternal() const;
    std::string commsCommonBigUnsignedValueNameBinSearchPairsInternal() const;
    std::string commsCommonValueNameBinSearchPairsInternal() const;

    ValidRangesList m_validRanges;
};

} // namespace commsdsl2new
