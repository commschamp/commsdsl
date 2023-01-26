//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>
#include <vector>

#include "commsdsl/parse/OptCond.h"
#include "XmlWrap.h"
#include "Logger.h"
#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

class OptCondImpl
{
public:
    using Ptr = std::unique_ptr<OptCondImpl>;
    using Kind = OptCond::Kind;
    using FieldsList = std::vector<FieldImplPtr>;

    OptCondImpl() = default;
    OptCondImpl(const OptCondImpl&) = default;
    OptCondImpl(OptCondImpl&&) = default;
    virtual ~OptCondImpl() = default;

    Kind kind() const
    {
        return kindImpl();
    }

    Ptr clone() const
    {
        return cloneImpl();
    }

    bool verify(const FieldsList& fields, ::xmlNodePtr node, Logger& logger) const
    {
        return verifyImpl(fields, node, logger);
    }

protected:
    virtual Kind kindImpl() const = 0;
    virtual Ptr cloneImpl() const = 0;
    virtual bool verifyImpl(const FieldsList& fields, ::xmlNodePtr node, Logger& logger) const = 0;
};

class OptCondExprImpl final: public OptCondImpl
{
public:
    OptCondExprImpl() = default;
    OptCondExprImpl(const OptCondExprImpl&) = default;
    OptCondExprImpl(OptCondExprImpl&&) = default;

    bool parse(const std::string& expr, ::xmlNodePtr node, Logger& logger);

    const std::string& left() const
    {
        return m_left;
    }

    const std::string& op() const
    {
        return m_op;
    }

    const std::string& right() const
    {
        return m_right;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual bool verifyImpl(const FieldsList& fields, ::xmlNodePtr node, Logger& logger) const override;

private:
    bool hasUpdatedValue();
    bool checkComparison(const std::string& expr, const std::string& op, ::xmlNodePtr node, Logger& logger);
    bool checkBool(const std::string& expr, ::xmlNodePtr node, Logger& logger);
    static FieldImpl* findField(
        const FieldsList& fields,
        const std::string& name,
        std::size_t& remPos);
    bool verifyBitCheck(const FieldsList& fields, ::xmlNodePtr node, Logger& logger) const;
    bool verifyComparison(const FieldsList& fields, ::xmlNodePtr node, Logger& logger) const;

    std::string m_left;
    std::string m_op;
    std::string m_right;
};

class OptCondListImpl final : public OptCondImpl
{
    using Base = OptCondImpl;
public:
    using Type = OptCondList::Type;
    using List = std::vector<Ptr>;
    using CondList = OptCondList::CondList;

    OptCondListImpl() = default;
    OptCondListImpl(const OptCondListImpl& other);
    OptCondListImpl(OptCondListImpl&&) = default;

    Type type() const
    {
        return m_type;
    }

    CondList condList() const;

    bool parse(::xmlNodePtr node, Logger& logger);



protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual bool verifyImpl(const FieldsList& fields, ::xmlNodePtr node, Logger& logger) const override;

private:
    List m_conds;
    Type m_type = Type::NumOfValues;
};

using OptCondImplPtr = OptCondImpl::Ptr;

} // namespace parse

} // namespace commsdsl
