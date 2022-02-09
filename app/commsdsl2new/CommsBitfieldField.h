#pragma once

#include "CommsField.h"

#include "commsdsl/gen/BitfieldField.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2new
{

class CommsGenerator;
class CommsBitfieldField final : public commsdsl::gen::BitfieldField, public CommsField
{
    using Base = commsdsl::gen::BitfieldField;
    using CommsBase = CommsField;
public:
    CommsBitfieldField(CommsGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // CommsBase overrides
    virtual IncludesList commsCommonIncludesImpl() const override;
    virtual std::string commsCommonCodeBodyImpl() const override;
    virtual std::string commsCommonMembersCodeImpl() const override;
    virtual IncludesList commsDefIncludesImpl() const override;
    virtual std::string commsDefMembersCodeImpl() const override;
    virtual std::string commsBaseClassDefImpl() const override;
    virtual std::string commsDefPublicCodeImpl() const override;
    // virtual std::string commsDefValidFuncBodyImpl() const override;

private:
    bool commsPrepareInternal();
    std::string commsFieldDefOptsInternal() const;
    std::string commsAccessCodeInternal() const;

    CommsFieldsList m_members;
};

} // namespace commsdsl2new
