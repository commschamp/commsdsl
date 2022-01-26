#pragma once

#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/Endian.h"

#include <string>

namespace commsdsl2new
{

class CommsGenerator;
class CommsField
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    using IncludesList = StringsList;

    explicit CommsField(commsdsl::gen::Field& field);
    virtual ~CommsField();

    bool commsPrepare();
    bool commsWrite() const;

    IncludesList commsCommonIncludes() const;
    std::string commsCommonCode() const;

    IncludesList commsDefIncludes() const;
    std::string commsDefCode() const;

    void setForcedFailOnInvalid()
    {
        m_forcedFailOnInvalid = true;
    }

    void setForcedPseudo()
    {
        m_forcedPseudo = true;
    }

    void setReferenced()
    {
        m_referenced = true;
    }

protected:
    virtual IncludesList commsCommonIncludesImpl() const;
    virtual std::string commsCommonCodeBodyImpl() const;
    virtual IncludesList commsDefIncludesImpl() const;
    virtual std::string commsDefMembersCodeImpl() const;
    virtual std::string commsDoxigenDetailsImpl() const;
    virtual std::string commsExtraDoxigenImpl() const;
    virtual std::string commsBaseClassDefImpl() const;
    virtual bool commsIsLimitedCustomizableImpl() const;

    std::string commsCommonNameFuncCode() const;
    bool commsIsVersionOptional() const;
    std::string commsFieldBaseParams(commsdsl::parse::Endian endian) const;
    void commsAddFieldDefOptions(commsdsl::gen::util::StringsList& opts) const;
    bool commsIsFieldCustomizable() const;

private:
    bool commsWriteCommonInternal() const;
    bool commsWriteDefInternal() const;
    std::string commsFieldDefCodeInternal() const;
    std::string commsOptionalDefCodeInternal() const;
    std::string commsFieldBriefInternal() const;
    std::string commsDocDetailsInternal() const;
    std::string commsExtraDocInternal() const;
    std::string commsDeprecatedDocInternal() const;
    std::string commsTemplateParamsInternal() const;

    commsdsl::gen::Field& m_field;
    std::string m_customRead;
    std::string m_customRefresh;
    std::string m_customWrite;
    bool m_forcedFailOnInvalid = false;
    bool m_forcedPseudo = false;
    bool m_referenced = true; // TODO: false by default
};

} // namespace commsdsl2new
