#pragma once

#include <string>
#include <vector>

#include "CommsdslApi.h"

namespace commsdsl
{

class OptCondImpl;
class COMMSDSL_API OptCond
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

class OptCondExprImpl;
class COMMSDSL_API OptCondExpr : public OptCond
{
    using Base = OptCond;
public:
    OptCondExpr(const OptCondExprImpl* impl);
    OptCondExpr(OptCond cond);

    const std::string& left() const;
    const std::string& op() const;
    const std::string& right() const;
};

class OptCondListImpl;
class COMMSDSL_API OptCondList : public OptCond
{
    using Base = OptCond;
public:
    enum class Type
    {
        And,
        Or,
        NumOfValues
    };

    using CondList = std::vector<OptCond>;

    OptCondList(const OptCondListImpl* impl);
    OptCondList(OptCond cond);


    Type type() const;
    CondList conditions() const;
};

} // namespace commsdsl
