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

protected:
    virtual IncludesList commsCommonIncludesImpl() const;

private:
    bool commsWriteCommon() const;
    bool commsWriteDef() const;

    commsdsl::gen::Field& m_field;
};

} // namespace commsdsl2new
