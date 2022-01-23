#pragma once

#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2new
{

class CommsGenerator;
class CommsField
{
public:
    using IncludesList = commsdsl::gen::util::StringsList;

    explicit CommsField(commsdsl::gen::Field& field);
    virtual ~CommsField();

    bool commsWrite() const;

    IncludesList commsCommonIncludes() const;
    std::string commsCommonCode() const;

    IncludesList commsDefIncludes() const;
    std::string commsDefCode() const;

protected:
    virtual IncludesList commsCommonIncludesImpl() const;
    virtual std::string commsCommonCodeBodyImpl() const;
    virtual IncludesList commsDefIncludesImpl() const;
    virtual std::string commsDefCodeImpl() const;

    std::string commsCommonNameFuncCode() const;
    bool commsIsVersionOptional() const;
    

private:
    bool commsWriteCommon() const;
    bool commsWriteDef() const;

    commsdsl::gen::Field& m_field;
};

} // namespace commsdsl2new
