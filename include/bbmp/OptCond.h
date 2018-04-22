#pragma once

#include <string>

#include "BbmpApi.h"

namespace bbmp
{

class OptCondImpl;
class BBMP_API OptCond
{
public:

    enum class Kind
    {
        Expr,
        List,
        NumOfValues
    };

    explicit OptCond(const OptCondImpl* impl);
    OptCond(const OptCond& other);
    ~OptCond();

    bool valid() const;

    Kind kind() const;

protected:
    const OptCondImpl* m_pImpl;
};

class OptCondExpr : public OptCond
{
public:

};

class OptCondList : public OptCond
{
public:
    enum class Type
    {
        And,
        Or,
        NumOfValues
    };

    Type type() const;
};

} // namespace bbmp
